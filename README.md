> [!IMPORTANT]
> **Test builds & discussion**
>
> [Join Flat2VR Modding](https://discord.gg/P4q9zt7ek) · [Assassin’s Creed VR thread](https://discord.com/channels/747967102895390741/1567686374251831416)
>
> Use the thread for test build availability and feedback.

# Assassin’s Creed VR

**Full motion controls in first person.** Fight and climb as Altaïr with tracked left and right hands, stereo OpenXR and 6DoF head tracking.

For the original **Assassin’s Creed (2007)** on Steam for Windows. Tested on Meta Quest 3.

<img src="https://github.com/user-attachments/assets/c473fe3b-a89f-4482-904a-7e8cdefdf2bb" width="400" alt="Assassin’s Creed VR in-game screenshot" />

## FEATURES

| | What you can do |
| :---: | --- |
| <img src="assets/feature-icons/sword.png" width="26" height="26" alt="" /> | **Sword combat.** Draw your sword from your hip or back. Swing faster to hit harder, raise the blade to block, parry and counter, or shove with an open palm. |
| <img src="assets/feature-icons/climb.png" width="26" height="26" alt="" /> | **Climb and pull yourself over ledges.** Grip with either hand and switch hands as you climb. Feel controller feedback on contact with the wall. |
| <img src="assets/feature-icons/hidden-blade.svg" width="26" height="26" alt="" /> | **Hidden blade and stealth kills.** Flick your wrist to extend the blade, then take down a guard from behind. |
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

The mod includes hand and head collision with scenery and improved NPC detail near the edge of the VR view. It is experimental, and the cape and legs can still appear in first person. If you hit a problem, share your headset, the scene and steps to reproduce it in [Issues](https://github.com/Hngamers191/AC1-VR-MOD/issues).

<details>
<summary>Native rebuild for contributors</summary>

From the x86 Native Tools Command Prompt for Visual Studio, run `tools\build_native.bat`, then `tools\REHASH.bat` from the mod folder.

</details>

## Credits

Mod by **Hngamers** and **Monsterdeadwood**.

**Sword of Eden | AC Unity** model by **LostOfSpace**, licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

Assassin’s Creed is a trademark of Ubisoft. This is an unofficial fan project.
