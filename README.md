[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![linux](https://github.com/jd28/arclight/actions/workflows/linux.yml/badge.svg)](https://github.com/jd28/arclight/actions?query=workflow%3Alinux)
[![macos](https://github.com/jd28/arclight/actions/workflows/macos.yml/badge.svg)](https://github.com/jd28/arclight/actions?query=workflow%3Amacos)
[![windows](https://github.com/jd28/arclight/actions/workflows/windows.yml/badge.svg)](https://github.com/jd28/arclight/actions?query=workflow%3Awindows)

# Arclight Tools

Tools subproject of something referred to as Arclight

**This is beta software, please, keep backups.**

### Platforms

- Linux: The latest LTS of Ubuntu, roughly
- macOS: macOS Monterey and above.  Need to set up code signing.
- Windows: Windows 10 and above, Visual Studio 2019 seems to be the latest Qt supported version

## The Tools

Even though the utlimate goal of this project is something tooset-ish.  There will be some tools added
that are usefully independently of a toolset context.  They

### erfherder
**status:** beta

erfherder is an editor for [BioWare's ERF File Format](docs/bioware_aurora_engine_file_formats/Bioware_Aurora_ERF_Format.pdf).  It's features include:

* Everything in nwhak.exe
* Drag and Drop support.
* Very simple, but effective Fuzzy Search filtering.

![erfherder](screenshots/erfherder-2022-03-27.gif)

### texview
**status:** beta

A super mid and basic texture gallery viewer for Bioware DDS, Standard DDS, TGA, and PLT texures.  It can open Key, Erf, and Zip containers.  Screenshot is using NWN:EE main key file circa 36-2.

![texview](screenshots/texi-2024-04-09.gif)

### dlg
**status:** alpha

An editor for DLG conversation files and rollnw DLG json files.  A project of this level of complexity requires a fair bit of polish.. It has most toolset features, the main missing things is:

* Since it exists outside of a module.. completion on module scripts and journal integration.
* Inserting action strings (and corresponding highlights)
* Loading a TLK file and handling dialog nodes with string references.

![dlg](screenshots/dlg-2024-05-01.gif)

-----------------------------------------------------------------------------

## The Renderer

Currently the built in renderer only supports basic models. Now that it has been converted
to using [DiligentEngine](https://github.com/DiligentGraphics/DiligentEngine) as the cross-platform
API, it will be easier to add new features.

Features:
* Meshses
* Skin Meshes
* (s)lerped Animations
* basic area viewing

Caveats:
* No parts based models yet, emmitters, dangly meshes, etc.
* Diligent uses HLSL which is different from what the game uses.

-----------------------------------------------------------------------------

## The Widgets

### Area View

A view for NWN areas. *Extremely* basic, but a start.

Camera Controls:
* WASD for movement.
* Up and Down arrow for height.
* Ctrl+Up and Ctrl+Down for pitch.
* Right and Left arrow for yaw.

![area](screenshots/area-view.png)

### Container View

A view of NWN containers.  See erfheder above.

### Creature View 

A widget for creatures

![view](screenshots/creature-view-2025-02-15.gif)

### Dialog View

A tree view for NWN dialogs.  See dlg above.

### Door View

A widget for doors

![door](screenshots/door-view-2025-02-11.png)

![door](screenshots/door-loadscreen-2025-02-11.png)

### Encounter View

A widget for encounters, for spawn lists current drag-n-drop to add, DEL to remove.

![encounter](screenshots/encounter-view-2025-02-26.png)

### Item View

A widget for items.

![item](screenshots/item-view-item-properties-2025-02-06.png)

![item](screenshots/item-view-per-part-coloring-2025-02-06.png)

![item](screenshots/item-view-simple-models-2025-02-06.png)

![item](screenshots/item-view-inventory-2025-02-06.gif)

### Placeable View

A widget for placeables

![placeable](screenshots/placeable-view-2025-02-09.png)

### Sound View

A widget for sounds. Double clicking sounds whether in the available
or the current list will play that sound.

![sound](screenshots/sound-view-2025-03-02.gif)

### Trigger View

A widget for triggers

![trigger](screenshots/trigger-view-2025-02-26.png)

### Variable Table View

A widget for editing variable tables.  Error indicators on duplicates,
warning indicator on string variables with values convertible to int or float.

![vartable](screenshots/vartable-view-2024-06-17.png)


## The Views

These are widgets that present different aspects of a module.  There is still a lot of work
and investigation to be done here.

![treeviews](screenshots/project_tree_views.png)

### The Project View

The project view is a filesystem view of the module.  This could be a flat single directory
like the NWN Toolsets "temp.0" folder.  Or it can be arbitrary, an error indicator is shown
if a file is shadowed by another, since in the end there is only the flat namespace of ERF
files.  This view might not be better than palette views or maybe in addition to them at
some point.

### The Area List View

This is the list of areas, similar to the NWN Toolset.  I don't see Dialogs as being particularly
special, and scripts have far far better external editors today than could ever be built internally,
there's no reason for a special scripts tree node.

### The Explorer View

This will be a view of the modules external dependencies presented very much as NWN explorer does.
Currently it only shows the modules HAK files.

## Acknowledgements, Credits, & Inspirations

- [BioWare](https://bioware.com), [Beamdog](https://beamdog.com) - The game itself
- [Qt](https://www.qt.io) - Gui
- [neveredit](https://github.com/sumpfork/neveredit) - Inspiration
- [NWNExplorer](https://github.com/virusman/nwnexplorer) - Inspiration
- [NWN Java 3d Model Viewer](https://neverwintervault.org/project/nwn1/other/nwn-java-3d-model-viewer) - Inspiration
