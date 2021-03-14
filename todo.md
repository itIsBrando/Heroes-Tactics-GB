# **To-Do List**

## Features:
- [ ] Unit spawning on map (**high**)
- [ ] Add support for more than 3 units per team (**low**)
- [ ] Replace `map select` with an open world map
- [ ] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` accept the size of the `heursitic_t` as a parameter
- [ ] have `ai_get_heursitic_target(unit_t *, heursitic_t *)` sort the `heursitic_t` array in order of smallest priority to largest priority (*HIGH*)

## Polish:
- [ ] Add graphical portrait on unit's HUD
- [ ] Add `<>` characters to the team selection screen
- [ ] Remove much whitespace from `match set up screen` 

## Bug Fixes:
- [ ] Fix newline graphical bug in `print()` function
- [ ] Fix units that spawn outside of the map
- [ ] Fix the unit's ability to move 1 tile additional outside its `movePoints` (**high**)
- [ ] Healers can move after healing (**high**)