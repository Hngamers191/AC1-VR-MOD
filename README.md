<p align="center">
  <img src="docs/img/hero.png" width="420" alt="First-person view from the mod: both of Altair's hands on a stone wall above a Jerusalem street">
</p>

<h1 align="center">Assassin's Creed VR (2007)</h1>

<p align="center">
  <img src="https://img.shields.io/badge/runtime-OpenXR-0C6B7D?style=flat-square" alt="Runtime: OpenXR">
  <img src="https://img.shields.io/badge/game-AC1%20DX10-0C6B7D?style=flat-square" alt="Game: AC1 DX10">
  <img src="https://img.shields.io/badge/headset-Quest%203%20tested-0C6B7D?style=flat-square" alt="Headset: Quest 3 tested">
  <img src="https://img.shields.io/badge/status-in%20development-8C5A12?style=flat-square" alt="Status: in development">
</p>

<p align="center"><b>First-person VR · 6DoF head tracking · full motion controls for both hands · stereo OpenXR</b></p>

<p align="center"><b><a href="https://hngamers191.github.io/AC1-VR-MOD/">Open the site →</a></b></p>

---

Masyaf and Jerusalem in a headset. Tracked hands, a sword you swing yourself, climbing
with your own grip — running on the original game through OpenXR.

Become Altair in full immersive VR with this Assassin's Creed VR port. Wield the hidden
blade and Altair's sword in first person, and be an assassin in the Holy Land of Jerusalem.

## What the mod adds

- **Tracked hands and body** — your own hands in first person, with live fingers and a body that moves under you
- **Physical sword combat** — swing it yourself; hits land where the blade lands, blocks come from your guard
- **Climb with your grip** — squeeze a ledge, pull down, reach for the next hold with your free hand
- **Hidden blade** — flick your wrist to extend it, stealth kills from behind on anyone unaware
- **In-VR settings menu** — Combat, Blade, Comfort, Extras and Mods tabs, floating in front of you
- **Swappable swords** — Sword of Eden, Sword of Altair v2, Altair's Short Blade from your back, and more
- **Play as Desmond, in the modern day** *(coming next)* — step out of the Animus and walk the present-day sections in VR

## Install and launch

For the original **Assassin's Creed (2007)** on Steam for Windows. The base VR mod has
been tested on Meta Quest 3.

1. Install the original **Assassin's Creed (2007)** from Steam on Windows. Use
   **Manage → Browse local files** in Steam to open the game folder.
2. Extract the test ZIP you received. Copy the complete mod folder containing `RUN.bat`
   into the game folder, next to the game's `.exe`.
3. Connect your headset and start its PC OpenXR runtime. Open the mod folder and run
   `RUN.bat` each time you play.

In the headset, click the right thumbstick to open the settings menu.

## Test notes

Headfirst falls, sword throwing and procedural weather are in active testing.

The mod includes hand and head collision with scenery and improved NPC detail near the
edge of the VR view. It is experimental, and the cape and legs can still appear in first
person. If you hit a problem, share your headset, the scene and steps to reproduce it in
[Issues](https://github.com/Hngamers191/AC1-VR-MOD/issues).

## Known issues

- **Stretching NPCs in third person.** People nearby can stretch or distort for a moment.
  It does not affect play.
- **Delays during combat.** Fights with several guards can slow down for a few seconds.
  That is normal for this build and it settles once the fight ends.

Found something else? Message the authors — open an
[issue](https://github.com/Hngamers191/AC1-VR-MOD/issues), or get in touch with HNgamers
or MonsterDeadwood directly. Say what you were doing and what happened, and send the
newest folder from `logs\runs`; that folder is what makes a fault findable.

## Credits

**The VR mod** — HNgamers, MonsterDeadwood

**Sword models** — Vortix (Sword of Eden), Yuri (Sword of Altair v2),
Milacetious (Altair's Short Blade)

**The game** — Assassin's Creed, Altair and the original designs are Ubisoft's.

> Ask each model author for permission and credit them before sharing this mod publicly.

## First-person sword test candidate

Use the AC1VR DX10 collaborator test package dated 2026-11-17 as the base. With the game closed, back up the files listed in [the overlay](sword-candidate), then copy the contents of that folder into the mod folder beside its existing RUN.bat, preserving subdirectories. Run `RUN.bat --check` there before launching. For headset play, start the headset's OpenXR runtime and run `RUN.bat` without simulator arguments.

The XR Sim replay recorded ordinary blade damage, an intercepted guard attack and a lethal counter against the same guard. Both processes exited cleanly and the encounter permissions and saves were restored. Physical-headset timing and feel remain unverified for this candidate. [Replay measurements](candidate-source/guard-replay.json)
