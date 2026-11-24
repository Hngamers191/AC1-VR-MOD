"""Admit the checkpoint guard and restore its temporary permission on shutdown."""
import math
import time


class Encounter:
    def __init__(self, session):
        self.h = session
        self.spawned = False
        self.target = None
        self.ready = False
        self.deadline = time.monotonic() + 105

    def tick(self):
        api = self.api = self.h.script.exports_sync
        body = api.capsule_status()
        if not body.get('hanging') or not api.first_person_status()['active']:
            return
        if not api.vr_world({'compact': True})['enabled']:
            api.vr_world({'action': 'ARM', 'compact': True})
        if self.ready:
            status = api.guard_grip()
            self.h.result['guard_grip'] = status
            if status['error']:
                raise RuntimeError(status['error'])
            return
        if self.h.sim and time.monotonic() > self.deadline:
            raise RuntimeError('Original guard did not become available')
        npc = api.npc_sword_lab()
        if not self.spawned:
            world = npc['activeWorld']
            ready = any(t.get('state') == 'ORIGINAL_FACTORY_READY' and t.get('physicsWorld') == world
                        and t.get('soldier') and t.get('nativeHumans') == 1 for t in npc['templates'])
            placement = api.native_player_placement()
            if not npc.get('valid') or not ready or not placement or placement['world'] != world:
                return
            heading = placement['rootYaw']
            root = placement['position']
            distance = 1.24
            position = [root[0] - distance * math.sin(heading),
                        root[1] + distance * math.cos(heading), root[2] + .04]
            orientation = [0, 0, math.sin((heading + math.pi) / 2), math.cos((heading + math.pi) / 2)]
            api.npc_sword_lab({'action': 'SPAWN_ONE_ORIGINAL_GUARD', 'position': position, 'quaternion': orientation})
            self.spawned = True
            self.h.result['guard_spawn'] = {'position': position, 'world': world, 'player': root}
            return
        if npc['errors']:
            raise RuntimeError(f'Original guard factory: {npc["errors"]}')
        if not self.target:
            if len(npc['owned']) != 1 or not any(a['entity'] == npc['owned'][0] for a in npc['live']):
                return
            self.target = npc['owned'][0]
            api.native_pair_probe({'action': 'ARM', 'target': self.target, 'compact': True})
            api.native_pair_probe({'action': 'REMOVE_ONE_OVERLAY', 'compact': True})
            return
        policy = api.native_pair_probe({'compact': True})
        if policy['errors']:
            raise RuntimeError(f'Original guard admission: {policy["errors"]}')
        if policy['policy']['removed']:
            api.guard_grip({'action': 'ARM', 'target': self.target})
            self.ready = True
            self.h.log('checkpoint_guard_ready', entity=self.target)

    def close(self):
        if not self.target:
            return
        api = self.h.script.exports_sync
        api.guard_grip({'action': 'STOP'})
        state = api.native_pair_probe({'action': 'RESTORE_ONE_OVERLAY', 'compact': True})
        deadline = time.monotonic() + 10
        while (state['policy']['removed'] or state['policy']['pending']) and time.monotonic() < deadline:
            time.sleep(.05)
            state = api.native_pair_probe({'compact': True})
        closed = not (state['policy']['removed'] or state['policy']['pending'])
        self.h.result['encounter_lease_closed'] = closed
        self.h.result['guard_policy_on_close'] = state['policy']
        self.h.result['guard_policy_errors_on_close'] = state['errors']
        if closed:
            api.native_pair_probe({'action': 'DISARM', 'compact': False})
            api.vr_world({'action': 'STOP', 'compact': True})
