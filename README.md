> [!IMPORTANT]
> **Test builds & discussion**
>
> [Join Flat2VR Modding](https://discord.gg/P4q9zt7ek) · [Assassin’s Creed VR thread](https://discord.com/channels/747967102895390741/1567686374251831416)
>
> Use the thread for test build availability and feedback.

# Assassin’s Creed VR

**Climb the walls. Jump from roof to roof. Dive headfirst off the edge.** Choose your blade, parry a guard’s strike, or throw your sword at them. Cross the city through a day–night cycle as rain, fog and wind roll in.

**First-person VR · 6DoF head tracking · full motion controls for both hands · stereo OpenXR**

For the original **Assassin’s Creed (2007)** on Steam for Windows. The base VR mod has been tested on Meta Quest 3.

<img src="https://github.com/user-attachments/assets/c473fe3b-a89f-4482-904a-7e8cdefdf2bb" width="400" alt="Assassin’s Creed VR in-game screenshot" />

## FEATURES

| | What you can do |
| :---: | --- |
| <img src="assets/feature-icons/climb.png" width="26" height="26" alt="" /> | **Parkour in first person.** Grab ledges with either hand and pull yourself up hand over hand. Feel controller feedback on contact with the wall. Jump between rooftops and dive headfirst over the edge. |
| <img src="assets/feature-icons/sword.png" width="26" height="26" alt="" /> | **Draw, duel and throw your sword.** Draw the blade from your hip or back. Swing faster to hit harder, raise the blade to block, parry and counter, shove with an open palm, or throw the sword at an enemy. |
| | **Choose your blade.** Pick Altaïr’s original sword, a curved saber, the Sword of Eden, or another sword model from the in-VR menu. |
| <img src="assets/feature-icons/hidden-blade.svg" width="26" height="26" alt="" /> | **Hidden blade and stealth kills.** Flick your wrist to extend the blade, then take down a guard from behind. |
| | **Day–night cycle and procedural weather.** Traverse the city from daylight to night as rain, fog and wind change around you. |
| <img src="assets/feature-icons/look.png" width="26" height="26" alt="" /> | **Wrist HUD.** Check health and the map by looking at your forearm. |
| <img src="assets/feature-icons/swap.png" width="26" height="26" alt="" /> | **Third-person VR.** Play as Desmond or Altaïr with gamepad-style movement, combat and Eagle Vision. |

## Install and launch

1. Install the original **Assassin’s Creed (2007)** from Steam on Windows. Use **Manage → Browse local files** in Steam to open the game folder.
2. Extract the test ZIP you received. Copy the complete mod folder containing `RUN.bat` into the game folder, next to the game’s `.exe`.
3. Connect your headset and start its PC OpenXR runtime. Open the mod folder and run `RUN.bat` each time you play.

## Controls

### First person

- Look around and lean with your headset. Use the movement stick to walk.
- Reach for a ledge, hold **Grip**, and pull down to climb. Alternate hands between holds.
- Use **Grip** at the hip or back holster to draw or stow your sword. Swing to strike, hold a guard pose to block, or push with an open palm.
- Flick your wrist to extend the hidden blade. Look at your forearm to check health and the map.

### Third person

- Use the movement and camera sticks for walking and looking.
- Use the gamepad-style actions to fight and activate Eagle Vision in the Desmond and Animus sections.

### Settings and sword models

Press **F8** on the PC keyboard to open the in-VR menu. Use **Combat**, **Blade**, **Comfort**, **Extras** and **Tune** to adjust settings, including snap or smooth turning, the comfort vignette, soft camera follow, and holster reach.

The **Mods** tab offers Altaïr’s original sword, the Studded Bronze Sword, Brass Broadsword, Curved Saber and Sword of Eden. The Sword of Eden’s engraved glow can be switched on or off. All models use the original sword’s combat and hit area.

## Test notes

Headfirst falls, sword throwing and procedural weather are in active testing.

The mod includes hand and head collision with scenery and improved NPC detail near the edge of the VR view. It is experimental, and the cape and legs can still appear in first person. If you hit a problem, share your headset, the scene and steps to reproduce it in [Issues](https://github.com/Hngamers191/AC1-VR-MOD/issues).

<details>
<summary>Native rebuild for contributors</summary>

From the x86 Native Tools Command Prompt for Visual Studio, run `tools\build_native.bat`, then `tools\REHASH.bat` from the mod folder.

</details>

## Credits

Mod by **Hngamers** and **Monsterdeadwood**.

**Sword of Eden | AC Unity** model by **LostOfSpace**, licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

Assassin’s Creed is a trademark of Ubisoft. This is an unofficial fan project.
