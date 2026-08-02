# [ THE GUILDHOUSE MANUAL ]
>Compiled by: Conan96   
>Version 0.1 - 27 July 2026

---------------------------------------
### Introduction
GuildHouse is a [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk/) wotlk mod  

This manual is created to officially state the purpose and explain the usage of GuildHouse system

This Addon will use a guild area in the world. Default predefined areas are GM Island and a map not in use broken down into 3 areas.  

---------------------------------------
### Features of the GuildHouse:  

- Purchase your own area to be used as a GuildHouse (via the Broker)  
- Purchase Items that can be placed anywhere in your GuildHouse (via the Salesman)  

---------------------------------------
### Things the Broker can do:

- Purchase, Sell a GuildHouse
- Teleport a player to the GuildHouse they own

---------------------------------------
### Added/In Progress

- Added Locations to use for guilds with boundries, teleports player back to port location if trying to go out of bounds
- Added Broker - Purchase/Sell Guild Hall, Teleport to Guild area (only GM or higher can place in game) **working**
- Added Salesperson - Purchase Items that can be placed in the guild area (Security of who in guild can access set by GM) **working**
- Added commands to Purchase/Sell/Place/Move/Store items For the guild area design (Security of who in guild can access set by GM) **working**
- Added phases **working**
    - Need to create temporary phase when available, per location and free up phase when last member leaves **(to do)**
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
    - Furnature
        - Chairs
        - Tables **(Completed not tested)**
        - Chests
        - Dressers
        - Bookcases
    - Indoor Decorations
    - Outdoor Decorations
        - Mailboxs **(Completed not tested)**
        - Braziers
        - Lamp Posts
    - Animals
  
---------------------------------------
### GuildHouse Commands
Note that some commands may not be available to all accounts (depending on their access level and permissions set in the RBAC tables (TC) / in the `command` table (AC)). You may need to change your account permissions / security level to enable usage of some commands
Most GuildHouse commands are divised by persmissions into groups: player commands and GM commands.
```
KEY:
< >  (angled brackets) indicates essential command argument  
[ ]  (square brackets) indicates optional command argument  
 |  (pipe character) indicates argument choices (i.e. this|that  = this OR that)  
_ARGUMENT_  indicates argument names  
```
**COMMAND**: **`.guildhouse | .gh`** -- (Player command) by itself will list all syntax available  

  - **`asset`** -- (Proper Guild Rank Access to use command)  
    - **`list`** -- Lists all owned assets in the world, and their asset ID and status of the asset.  
            **Example Usage**:  
                - `.gh list`  
    - **`move <_ASSETID_>`** -- Move a spawned asset in the world from where it was placed to the players current position.  
        - _ASSETID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh move 2`  
    - **`place <_ASSETID_>`** -- Place a specific non spawned asset at the players current position.  
        - _ASSETID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh place 1`  
    - **`sell <_ASSETID_>`** -- Sell a specific asset, removing it from guild inventory.  
        - _ASSETID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh sell 5`  
    - **`store <_ASSETID_>`** -- Removes a spawned asset from the world, saving it to be spawned at a future time.  
        - _ASSETID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh store 12`  

  - **`house`** -- (Proper Guild Rank Access to use command)  
    - **`sell`** -- Sells the house and all its contents back to the broker, refund is a set percentage of cost.  
            **Example Usage**:  
                - `.gh house sell`  

  - **`house`** -- (Any player in a guild that owns a house)  
    - **`tele | teleport`** -- Teleports the player to the guild house they own.  
            **Example Usage**:  
                - `.gh house tele`  

  - **`npc`** -- (Game Master or higher)  
    - **`broker`** -- Spawns a broker at the players position. There can be more then one.  
            **Example Usage**:  
                - `.gh npc broker`  

  - **`npc`** -- (Proper Guild Rank Access to use command)  
    - **`salesman`** -- Spawns a salesman at the current location inside the guild area. There can only be one.  
            **Example Usage**:  
                - `.gh npc salesman`  

  - **`shop`** -- (Proper Guild Rank Access command)  
    - **`buy <_CATALOGID_>`** -- Purchase a specific item  
        - _CATALOGID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh shop buy`  
    - **`categories`** -- List all categories  
            **Example Usage**:  
                - `.gh shop categories`  
    - **`list <_CATEGORYID_>`** -- Lists all items in a category  
        - _CATEGORYID_ = specific owned asset in your list  
            **Example Usage**:  
                - `.gh shop list 100`  

