# **To-Do List**

## Features:
- [ ] Unit spawning on map (**high**)
- [ ] Add support for more than 3 units per team (**low**)
- [ ] Replace `map select` with an open world map
- [ ] Add CGB background coloration
- [ ] Add CGB sprite coloration
- [ ] Detect winning/losing condition immediately after battle, not at the end of a turn
- [ ] Add fog (**high**)
- [ ] Make fog optional
- [ ] Add title screen (**high**)
- [ ] Link cable multiplayer (**low**)
- [x] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` accept the size of the `heursitic_t` as a parameter
- [x] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` sort the `heursitic_t` array in order of smallest priority to largest priority (**high**)

## Polish:
- [ ] Remove much white-space from *match set up screen*
- [ ] Water animation
- [ ] Improve visuals on the banner that appear when changing turns
- [ ] Add game over screen
- [ ] Add a visual on the HUD to indicate unit selection
- [ ] Add how-to screen (**low**)
- [x] Add `<>` characters to the *team selection screen*
- [x] Add graphical portrait on unit's HUD

## Bug Fixes:
- [ ] Fix newline graphical bug in `print()` function
- [ ] Fix units that spawn outside of the map
- [ ] Fix the unit's ability to move 1 tile additional outside its `movePoints` (**high**)
- [ ] Healers can move after healing (**high**)
- [ ] Remove debug function `get_strat_string` in `hud.c`. This function shows the AI's strategy