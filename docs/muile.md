

# MUiLE
Here: https://docs.google.com/presentation/d/1Ra4I0l1IFDMQLiNT7jBDehKVO-MrKICt/edit#slide=id.p28
This presentation has a lot of info on the layout of SimCity.
Throughout the game files are json files that look much like the slide linked.
These are in MUiLE format.

```
# I will be using a # to signify annotations as comments don't exist in JSON.
{
    instanceID: integer, # Unique ID for the UI object

    # The following values represent the x, y, width, and height respectively.
    left: integer,
    top: integer,
    width: integer,
    height: integer,

    # Unknown.
    horizontalPinType: integer,
    verticalPinType: integer,

    # Whether or not the object is visible.
    visibility: boolean,

    # Whether or not the object doesn't process mouse input. (?)
    ignoreMouse: boolean,
    
    # The children of the UI object. Children are offset by the position of their parent.
    children: [muileObject],

    # The type of the UI object.
    type: string,

    # version. (?)
    version: number
}
```

`version` is always 1 and is only in the top level UI object.
