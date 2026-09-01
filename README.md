# [ THE GUILD ENCLAVE MANUAL ]
>Compiled by: Conan96   
>Version 0.2 - 11 Aug 2026

---------------------------------------
### Introduction
GuildEnclave is a [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk/) wotlk mod  

This manual is created to officially state the purpose and explain the usage of GuildEnclave system

This Addon will use a guild area in the world. Default predefined areas are GM Island and a map not in use broken down into 3 areas.  

---------------------------------------
### Features of the GuildEnclave:  

- Purchase your own area to be used as a GuildEnclave (via the Broker)  
- Purchase Items that can be placed anywhere in your GuildEnclave (via the Salesman)  
- Separate phases for each guild in that specific area. This way each design is different.  
- Dynamic loading of Enclave when a member enters the guild, and released when the last member leaves.  
- Every guild purchase also adds a "Salesman" as your first asset, this can not be sold.
- There is a default asset package for each guild location that can be purchased and customized for your guild.  

---------------------------------------
### Things the Broker can do:

- Purchase, Sell a GuildEnclave
- Teleport a player to the GuildEnclave they own

---------------------------------------
###  How to use:  

- See the Broker to purchase an Enclave if you have a Guild
- Teleport to the guild via command or the Broker
- place your Guild Salesman anywhere you like within the purchased guild via the command `.ge asset place 1`
- Purchase items via the Salesman and place, move, or store them for later use.

---------------------------------------
### Build Catalog Items
These can only be added via Game Masters and they are used to create a GuildEnclave Package via `build` command.  
The correct faction object will spawn for the guild using the build.  
These are also located on the Salesman or via chat commands for purchase.  

<DETAILS>
<SUMMARY>Class Trainers</SUMMARY>

- 1101: Death Knight Trainer  
- 1102: Druid Trainer  
- 1103: Hunter Trainer  
- 1104: Mage Trainer  
- 1105: Paladin Trainer  
- 1106: Priest Trainer  
- 1107: Rogue Trainer  
- 1108: Shaman Trainer  
- 1109: Warlock Trainer  
- 1110: Warrior Trainer  

</DETAILS>
<DETAILS>
<SUMMARY>Profession Trainers</SUMMARY>

- 1201: Alchemy Trainer  
- 1202: Blacksmithing Trainer  
- 1203: Enchanting Trainer  
- 1204: Engineering Trainer  
- 1205: Inscription Trainer  
- 1206: Jewelcrafting Trainer  
- 1207: Leatherworking Trainer  
- 1208: Tailoring Trainer  
</DETAILS>
<DETAILS>
<SUMMARY>Gathering Trainers</SUMMARY>

- 1301: Herbalism Trainer  
- 1302: Mining Trainer  
- 1303: Skinning Trainer  

</DETAILS>
<DETAILS>
<SUMMARY>Secondary Profession Trainers</SUMMARY>

- 1401: Cooking Trainer  
- 1402: Fishing Trainer  
- 1403: First Aid Trainer  

</DETAILS>
<DETAILS>
<SUMMARY>Townsfolk</SUMMARY>

- 1501: Innkeeper  
- 1502: Banker  
- 1503: Auctioneer  
- 1504: Neutral Auctioneer  

</DETAILS>
<DETAILS>
<SUMMARY>Vendors</SUMMARY>

- 1601: Trade Supplies  
- 1602: Tabard Vendor  
- 1603: Food & Drink Vendor  
- 1604: Reagent Vendor  
- 1605: Ammo & Repair Vendor  
- 1606: Poisons Vendor  

</DETAILS>
<DETAILS>
<SUMMARY>Guards</SUMMARY>
</DETAILS>
<DETAILS>
<SUMMARY>Common Portals</SUMMARY>

- 2101: Stormwind Portal / Orgrimmar Portal  
- 2102: Ironforge Portal / Thunder Bluff Portal  
- 2103: Darnassus Portal / Undercity Portal  

</DETAILS>
<DETAILS>
<SUMMARY>Expansion Portals</SUMMARY> 

- 2201: Shattrath Portal  
- 2202: Dalaran Portal  
- 2203: Caverns of Time Portal  
- 2204: Blasted Lands Portal  

</DETAILS>
<DETAILS>
<SUMMARY>Chairs</SUMMARY>

- 3101: Wooden Chair  
- 3102: High Back Chair  
- 3103: Wooden Chair  
- 3104: Westfall Chair  
- 3105: Wooden Chair  
- 3106: Dwarven High Back Chair  
- 3107: Craig's Chair  
- 3108: Chair  
- 3109: Chair  
- 3110: Chair  
- 3111: Chair  
- 3112: Chair  
- 3113: Stone Chair  
- 3114: Stone Chair  
- 3115: Stone Chair  
- 3116: Chair  
- 3117: Chair  
- 3118: The Chair  
- 3119: Chair  
- 3120: Chair  
- 3121: Chair  
- 3122: Chair  
- 3123: Chair  
- 3124: Chair  
- 3125: Fishing Chair  
- 3126: Thane Ufrang's Chair  
- 3127: Barbershop Chair  
- 3128: Chair  
- 3129: Chair  
- 3130: Chair  
- 3131: Chair  

</DETAILS>
<DETAILS>
<SUMMARY>Tables</SUMMARY>

- 3201: Camp Table  
- 3202: WotLK Light Table  
- 3203: Cooking Table  
- 3204: Inn Table, Tiny  
- 3205: Dwarven Table, Ornate  
- 3206: WotLK Light Table, Small  
- 3207: Horde War Table  
- 3208: Elven Wooden Table  
- 3209: Dwarven Table, Small  
- 3210: Orc Table  
- 3211: Blood Elf Table, Small  
- 3212: Apothecary Table, Round  
- 3213: Ironridge Table  
- 3214: Alchemy Table  
- 3215: Table  
- 3216: Onslaught Table  
- 3217: Dwarven Table  
- 3218: Gnome Table  

</DETAILS>
<DETAILS>
<SUMMARY>Chests</SUMMARY>


</DETAILS>
<DETAILS>
<SUMMARY>Dressers</SUMMARY>


</DETAILS>
<DETAILS>
<SUMMARY>Bookcases</SUMMARY>


</DETAILS>
<DETAILS>
<SUMMARY>Mailbox</SUMMARY>

- 4101: Stormwind Mailbox  
- 4102: Orgrimmar Mailbox  
- 4103: Bouncing Mailbox  
- 4104: Dalaran Mailbox
- 4105: Totem Mailbox  
- 4106: Thunder Bluff Mailbox  
- 4107: Ironforge Mailbox  
- 4108: Darnassus Mailbox  
- 4109: The Exodar Mailbox  
- 4110: Silvermoon Mailbox  
- 4111: Undercity Mailbox  

</DETAILS>
<DETAILS>
<SUMMARY>Braziers</SUMMARY>


</DETAILS>
<DETAILS>
<SUMMARY>Lamp Posts</SUMMARY>

- 4301: Dalaran Lamp Post  

</DETAILS><DETAILS>
<SUMMARY>Animals</SUMMARY>


</DETAILS>
   
---------------------------------------  
### GuildEnclave Commands
Note that some commands may not be available to all accounts (depending on their access level and permissions set in the RBAC tables (TC) / in the `command` table (AC)). You may need to change your account permissions / security level to enable usage of some commands
Most GuildEnclave commands are divised by persmissions into groups: player commands and GM commands.
```
KEY:
< >  (angled brackets) indicates essential command argument  
[ ]  (square brackets) indicates optional command argument  
 |  (pipe character) indicates argument choices (i.e. this|that  = this OR that)  
_ARGUMENT_  indicates argument names  
```
**COMMAND**: **`.guildenclave | .ge`** -- (Player command) by itself will list all syntax available  

### All Guild Member or Game Master commands:  
  - **`enclave`**
    - **`tele | teleport`** -- Teleports the player to the guild enclave they own.  
      **Example Usage**:  
        - `.ge enclave tele`  

### Proper Guild Rank or Game Master commands:  
  - **`asset`**
    - **`list`** -- Lists all owned assets in the world, and their asset ID and status of the asset.  
      **Example Usage**:  
        - `.ge asset list`  
    - **`move <_ASSETID_>`** -- Move a spawned asset in the world from where it was placed to the players current position.  
      - _ASSETID_ = specific owned asset in your list  
      **Example Usage**:  
        - `.ge asset move 2`  
    - **`place <_ASSETID_>`** -- Place a specific non spawned asset at the players current position.  
      - _ASSETID_ = specific owned asset in your list  
      **Example Usage**:  
        - `.ge asset place 1`  
    - **`sell <_ASSETID_>`** -- Sell a specific asset, removing it from guild inventory.  
      - _ASSETID_ = specific owned asset in your list  
      **Example Usage**:  
        - `.ge asset sell 5`  
    - **`store <_ASSETID_>`** -- Removes a spawned asset from the world, saving it to be spawned at a future time.  
      - _ASSETID_ = specific owned asset in your list  
      **Example Usage**:  
        - `.ge asset store 12`
  
  - **`enclave`**
    - **`sell`** -- Sells the enclave and all its contents back to the broker, refund is a set percentage of cost.  
      **Example Usage**:  
        - `.ge enclave sell`  
  
  - **`shop`**
    - **`buy <_CATALOGID_>`** -- Purchase a specific item  
      - _CATALOGID_ = specific owned asset in your list  
      **Example Usage**:  
          - `.ge shop buy`  
    - **`categories`** -- List all categories  
      **Example Usage**:  
        - `.ge shop categories`  
    - **`list <_CATEGORYID_>`** -- Lists all items in a category  
      - _CATEGORYID_ = specific owned asset in your list  
      **Example Usage**:  
        - `.ge shop list 100`  
  
  - **`npc`**
    - **`wander <_ASSETID_> <_DISTANCE_>`** -- Change how far the asset will wander from its spawn point. Only usable on NPCs.  
      - _ASSETID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      - _DISTANCE_ = How far from spawn point the NPC will wander randomly  
      **Example Usage**:  
        - `.ge npc wander 1 50`
  
### Game Master or higher commands:  
  - **`npc`**
    - **`broker`** -- Spawns a broker at the players position. There can be more then one.  
      **Example Usage**:  
        - `.ge npc broker`  
  
  - **`enclave`**
    - **`list`** -- Lists all enclaves and if enabled/disabled.  
      **Example Usage**:  
        - `.ge enclave list`  
    - **`new "<_NAME_>" <_MAPID_> <_ZONEID_> <_AREAID_>`** -- Creates a new enclave, returning the _LOCATIONID_, also found by using the `.ge enclave list` command.  
      - _NAME_ = Unique name of new enclave.  
      - _MAP_ = MapId from .gps command.  
      - _ZONE_ = ZoneId from .gps command.  
      - _AREA_ = AreaId from .gps command.  
      **Example Usage**:  
        - `.ge enclave new "Goldshire" 0 12 87`  
    - **`setspawn <_LOCATIONID_> <_POSITIONX_> <_POSITIONY_> <_POSITIONZ_> <_ORIENTATION_>`** -- Sets the spawn location for the player when entering.  
      - _LOCATIONID_ = Id aquired by `.ge enclave list` or the returned id when created.  
      - _POSITIONX_ = X position.  
      - _POSITIONY_ = Y position.  
      - _POSITIONZ_ = Z position.  
      - _ORIENTATION_ = Orientation.  
      **Example Usage**:  
        - `.ge enclave setspawn 4 -9448.42 67.06755 56.22756 3.2`  
    - **`setboundaries <_LOCATIONID_> <_MINX_> <_MINY_> <_MAXX_> <_MAXY_>`** -- Set the boundaries of the GuildEnclave. If anyone leaves the boundary, they are teleported back in.  
      - _LOCATIONID_ = Id aquired by `.ge enclave list` or the returned id when created.  
      - _MINX_ = X position.  
      - _MINY_ = Y position.  
      - _MAXX_ = Z position.  
      - _MAXY_ = Z position.  
      **Example Usage**:  
        - `.ge enclave setboundaries 4 -9440 56 -9400 80`  
    - **`enable <_LOCATIONID_>`** -- Enable the guildenclave so it can be used. They are disabled by default when created.  
      - _LOCATIONID_ = Id aquired by `.ge enclave list` or the returned id when created.  
      **Example Usage**:  
        - `.ge enclave enable 4`  
    - **`disable <_LOCATIONID_>`** -- Disable the guildenclave so it can not be used. This is the default on a newly created guildenclave.  
      - _LOCATIONID_ = Id aquired by `.ge enclave list` or the returned id when created.  
      **Example Usage**:  
        - `.ge enclave disable 4`  
  
  - **`gameobjects`**  
    - **`new <_CATALOGID_> <_CATEGORYID_> <_OBJECTID_> <_FACTIONID_> <_COST_>`** -- Adds a gameobject into the purchaseable catalog.  
      - _CATALOGID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      - _CATEGORYID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      - _OBJECTID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      - _FACTIONID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      - _COST_ = specific owned asset in your list (viewed by `.ge asset list`)  
      **Example Usage**:  
        - `.ge gameobjects new 100 1035 5190 4 500000`  
  
  - **`build`**
    *note: used for designing or changing the default design of a GuildEnclave that can be purchased.
    - **`add <_CATALOGID_>`** -- Add an asset and spawns it at the individuals location at no cost.  
      - _CATALOGID_ = Specific catalog asset listed in the above list.  
      **Example Usage**:  
        - `.ge build add 2101` -- Adds and spawns either a Stormwind or Orgrimmar portal depending on faction.
    - **`clear`** -- Despawns all assets and removes them from your list.  
      **Example Usage**:  
        - `.ge build clear`
    - **`load`** -- Clears your asset list, then loads the default assets for the current GuildEnclave location placing them in the original position.  
      **Example Usage**:  
        - `.ge build load`
    - **`remove <_ASSETID_>`** -- Despawns (if spawned) and removes a specific asset from your design.  
      - _ASSETID_ = specific owned asset in your list (viewed by `.ge asset list`)  
      **Example Usage**:  
        - `.ge build remove 4`
    - **`save`** -- Saves all assets and changes to the build for the specific GuildEnclave location. Previous saved build will be overwritten.  
      **Example Usage**:  
        - `.ge build save`
  
---------------------------------------
### Added/In Progress

- Added Locations to use for guilds with boundries, teleports player back to port location if trying to go out of bounds
- Added Broker - Purchase/Sell Guild Hall, Teleport to Guild area (only GM or higher can place in game) **(working)**
- Added Salesperson - Purchase Items that can be placed in the guild area (Security of who in guild can access set by GM) **(working)**
- Added commands to Purchase/Sell/Place/Move/Store items For the guild area design (Security of who in guild can access set by GM) **(working)**
- Added temporary phases **(working)**
- Added build chat menus for Game Masters and functionality **(working)**
- Added default purchaseable package of assets for the guild you purchased. **(working)**
- Work on:
    - NPC
        - Class Trainers **(Completed not tested)**
        - Manufacturing Trainers **(Completed not tested)**
        - Gathering Trainers **(Completed not tested)**
        - Secondary Trainers **(Completed not tested)**
        - Townsfolk
        - Vendors
        - Guards
    - Portals
        - Common Portals **(Completed not tested)**
        - Expansion Portals **(Completed not tested)**
        - Custom Portals
    - Indoor Decorations
        - Chairs **(Completed not tested)**
        - Tables **(Completed not tested)**
        - Chests
        - Dressers
        - Bookcases
    - Outdoor Decorations
        - Mailboxs **(Completed not tested)**
        - Braziers
        - Lamp Posts
    - Animals

