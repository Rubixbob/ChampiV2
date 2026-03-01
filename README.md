# Champi

Champi is a Final Fantasy XIV gear solver that compares combinations of gear, materia and food and returns the one with the highest damage.

Results can exported to XivGear for visualisation, edition and sharing. Champi is only a solver.

## Quick start
1. Run `Champi.exe` (Windows 10/11 recommended).
1. In the `Solve` tab:
   - Choose a `Job` from the dropdown.
   - In the `Gear` sub-tab, select the iLvl range then select gear pieces for each equipment slot.
   - In the `Food` sub-tab, select the iLvl range then select the food item(s) you want the solver to consider.
   - Click `Solve` to start. A progress bar and estimated remaining time appear in the `Results` sub-tab.
1. When the solver finishes, select a result to view detailed melds and use the `XivGear Export` or `XivGear Export All` button to copy a JSON to your clipboard that can be imported in [xivgear](https://xivgear.app).

- Skill speed, Spell speed, Tenacity and Piety can be restricted to a certain range. This can speed up the solver and allow new results to appear in particular with increasing minimum piety.
- The `Damage Mod` number shown is the expected average damage for a 100 potency skill.
- Some settings can be tweaked from the `Settings` tab but it's recommended to keep the default values.

## Roadmap
- XivGear export ✔
- Pentamelds ✔
- SkS/SpS/Ten/Pie range ✔
- Saving sets
- Prog mode (solve with a limit on available tomestones and upgrade items)
- Make compatible (select multiple sets from the same job or different jobs, and try to fill the common pieces with the same materia)
- Lvl/iLvl sync mode (+ something to work with relics)
- Read files from game installation