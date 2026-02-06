# Gray Batch: RE merchants (partial safe subset)

## Scope
Attempted full conversion of `npc/re/merchants`, then switched only safe entries in `npc/re/scripts_athena.conf`.

## Conversion Result
Batch report:
- `.codex_tmp/re_merchants_batch1/report.md`

Summary:
- `total_files: 59`
- `unsupported_items: 42` (not all files are safe yet)

## Switched to Lua
47 entries were switched from `.txt` to `.lua` (safe subset: no unsupported + loadfile pass), including:
- `3rd_trader`, `advanced_refiner`, `alchemist`, `ammo_boxes`, `card_separation`, `cashmall`, `catalog`, `clothing_buff_removal`, `coin_exchange`, `Dealer_Update`, `diamond`, `Emperium_Seller`, `enchan_edda_half_moon`, `enchan_illusion_dungeons`, `enchan_illusion_17_1`, `enchan_ko`, `enchan_mal`, `enchan_sage_legacy_17_2`, `enchantgrade`, `Extended_Ammunition`, `Extended_Stylist`, `flute`, `Gemstone_Bagger`, `gld_mission_exchange`, `ghost_palace_exchange`, `guild_warehouse`, `hd_refiner`, `HorrorToyFactory_merchants`, `InfiniteSpace_merchants`, `moro_cav_exchange`, `mysterious_cookie_shop`, `new_insurance`, `nightmare_biolab`, `OldGlastHeim_merchants`, `pet_groomer`, `quests_exp_175`, `quivers`, `refine`, `renters`, `rgsr_in`, `shops`, `Slot_Move_Card_Sales`, `socket_enchant2`, `enchan_upg`, `te_merchant`, `shadow_refiner`, `eden_market`.

## Kept as TXT (deferred)
7 entries remain `.txt` due unsupported syntax or loadfile failures:
- `bio4_reward.txt`
- `enchan_mora.txt`
- `enchan_rockridge.txt`
- `enchan_verus.txt`
- `malangdo_costume.txt`
- `novice_vending_machine.txt`
- `pet_trader.txt`

## Build Validation
- `make -j4` passed

## Notes
- `./map-server` was not executed by Codex in this step.
