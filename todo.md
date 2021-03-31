# **To-Do List**

## Features:
- [ ] Add support for more than 3 units per team (**low**)
- [ ] Replace `map select` with an open world map
- [ ] Detect winning/losing condition immediately after battle, not at the end of a turn
- [ ] Allow player to press `B` button after it has moved (but still has attack diamond shown) to return unit to its position pre-movement (**high**)
- [ ] Add movement arrows that track the cursor (*optional*)
- [ ] Add campaign world map
- [ ] Link cable multiplayer (**low**)
- [x] Hold **`B`** to view attack range
- [x] Unit spawning on map (**high**)
- [x] Add fog
  - [x] Make fog optional
- [x] Add title screen
- [x] Add CGB background coloration
- [x] Add CGB sprite coloration
- [x] Allow player to attack an enemy without moving
- [x] FORCE UNIT TO ATTACK IMMEDIATELY AFTER MOVING
- [x] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` accept the size of the `heursitic_t` as a parameter
- [x] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` sort the `heursitic_t` array in order of smallest priority to largest priority

## Polish:
- [ ] Remove much white-space from *match set up screen*
- [ ] Water animation
- [ ] Improve visuals on the banner that appear when changing turns
- [ ] Add game over screen
- [ ] Add a visual on the HUD to indicate unit selection
- [ ] Change palette for house and bridge
- [ ] Add how-to screen (**low**)
- [x] Add CGB support to `unit_engage`
- [x] Add the unit's name above it in a battle
- [x] Add the stats of the two units to the battle screen
  - [x] Create icons
- [x] Change the palette of a unit that has moved
- [x] Add visual indicating which team is active
  - [x] colorize it
- [x] Add `<>` characters to the *team selection screen*
- [x] Add graphical portrait on unit's HUD

## Bug Fixes:
- [ ] Fix newline graphical bug in `print()` function
- [ ] Enemy units can be seen during movement when fog is enabled (**HIGH**)
- [ ] Remove `useFog` static variable in `menu.c`
  - this was used as a hacky way to enable fog on any map
- [ ] Fix units that spawn outside of the map
- [ ] Remove debug function `get_strat_string` in `hud.c`. This function shows the AI's strategy
- [ ] Palettes during unit healing is messed up (**HIGH**)
- [ ] Healers do not switch to `AI RUN`, instead stay on`HEAL`
- [x] AI run crashes on the 5th map (**HIGH**)
- [x] Enemy units can be exposed by peaking at tiles in fog
- [x] Healer must move otherwise it cannot heal an ally
- [x] Fix the unit's ability to move 1 tile additional outside its `movePoints`
- [x] Healers can move after healing