# GameplayConfiguration.md

There are a number of new components that are part of the MPS Setup. Within these components you can adjust many aspects of the game, including length of match, respawn penalties, and how and when certain gems spawn.  
  
**Match Controller Component Configuration**

![Matchcontroller_1](https://user-images.githubusercontent.com/67011188/221019125-f59a1be3-a025-4760-a4ea-adeb2cd2bd3b.JPG)
![network Match config](https://user-images.githubusercontent.com/67011188/221019269-95e7f010-1139-446f-b093-3f8b2f0074a7.JPG)

**Network Match**  
In this component you will set the length of the match and the point penalty applied to each character that is forced to respawn during the round.  

-   **Round Duration** - Here you will set the duration of each round.
-   **Total Rounds** - Here you will set the number of rounds. When all rounds are completed, the match ends, so this serves to dictate the length of each match as well.
-   **Respawn Penalty Percentage** - This sets the percentage of the player’s accrued points that are lost when they are forced to respawn, either by losing all of their shields or falling out of the level. This number can be set to 0 if desired, eliminating any respawn penalty.

![Gem Spawner](https://user-images.githubusercontent.com/67011188/221019308-e5dca564-6df5-4ea0-a8b1-c1d7fe326017.JPG)

**Gem Spawner**  
In this component you can set up the gem types, gem point values, and set which gems spawn in each round.  
-   **Gem Spawnables -** Here you will assign a tag, select an asset for the gem, and set the number of points that are awarded when the gem is picked up.
	-   **Tag** - Gem Tag is determines the gem type to spawn. Gem tags are used in per round spawn tables as well as individual gem spawn points.
	-   **Asset** - Here you will assign the asset to use for the gem.
	-   **Score** - Here you will set the point value for each gem.

-   **Spawn Tables Per Round** - Here you will use Gem Tags from the gem spawnables table to setup gem spawns for each game round. Starting with round [1] you can set the type of gems that spawn during that round. You can use all gem types, or a subset. Note: Spawn Table [0] is not used.
	-   **Gem Tag Type** - Using the Gem Tag this specifies the gem that spawns
	-   **Gem Weight** - In the event the spawn point (see below) calls two or more different Gem Tags, this value will influence which of those gems spawn.
	-   **Gem Spawn Tag** - This is used in Gem Spawn Points, see below.

![Gem Spawn Point](https://user-images.githubusercontent.com/67011188/221019364-5567ec1d-3697-4cf0-ac34-0d2e26e1efc7.JPG)

**Gem Spawn Points**  
Gem Spawn Points should be created wherever you want a gem to appear in the level.  

-   **Transform** - The location the gem will appear in the level.
-   **Tag** - Here you will enter the tags that correspond with the gems you want to spawn in this location. Entry [0] should be the Gem Spawn tag (set above in the Gem Spawn Tables Per Round component). Additional entries correspond with the gem(s) type you want to spawn in this location. You can add more than one gem type here. When this happens, the Gem Weight will influence which gem spawns each time. e.g. If you add yellow and green gems and give each a weight of 1, there will be an even chance of either to spawn. If one is set to greater than 1, that gem is more likely to spawn.
-   **Script Canvas & Mesh & Material components** - By default, gem spawn points have no visual indicator in the editor as gems only appear in those positions when the game is running. To allow you to visualize gem spawn points in the editor we add the mesh component and assign the gem model and a material that matches the gem that will appear in game. Then, when the game starts, we hide the placeholder gem models using the script canvas component

## **Setting Up Teleporters**

Teleporters are another new feature introduced in the MPS. These can be setup to teleport players anywhere in the level when they pass through the teleport volume.

![Teleport_01](https://user-images.githubusercontent.com/67011188/221019549-66964ea6-61b8-4150-8776-3b7587930078.JPG)

**Add the Teleporter Platform asset to the level**  

1.  In the asset browser, navigate to `\o3de-multiplayersample-assets\Gems\level_art_mps\Assets\Teleporter_Platform`
2.  Select the `Teleporter.prefab` and drag it into the level at the location you want it to appear.

![Network Exit](https://user-images.githubusercontent.com/67011188/221019649-06dba518-e20d-4643-b841-e9787ebb9e34.JPG)

**Add the Teleport Enter Volume**  

1.  Create a multiplayer entity in the level near the teleport platform and name it. Including the word ‘enter’ in the title is helpful in order to differentiate it from the exit point.
2.  Add the network teleport component to the multiplayer entity.
3.  Add the PhysX collider component and its dependency, the PhysX static rigid body component to the multiplayer entity.
4.  In the PhysX collider component, turn the Trigger option on and then choose the box shape. Set the box dimensions to 0.5, 1, 2. This will fit snuggly into the Teleport Platform doorway. Turn on Draw Collider, so you can see the volume.
5.  Align the volume with the Teleport Platform doorway.

![Teleporter Exit](https://user-images.githubusercontent.com/67011188/221019704-f57cb7e1-522e-4808-b005-4acd66613103.JPG)

**Add the Teleport Exit point**  

1.  Create an entity in the location you want the teleporter to exit and name it. Again, using the work ‘exit’ can be helpful.

**Link the Teleport Enter Volume with the Teleport Exit Point**  

1.  Select the Teleport Enter Volume.
2.  In the Network Teleport Component, hit the target reticle symbol and then select the Teleport Exit Point in the entity outliner.

You should now have a functioning teleporter! This describes the standard teleport setup, but feel free to experiment with the location and size of the teleport volumes.
