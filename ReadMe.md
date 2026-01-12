# ARKitCurveMapper
Purpose of this unreal plugin is to make it easier to map ARKit values from live link face onto any avatar. Usually avatar creators provide the mapping, but it often doesn't work for whatever reason. So this approach should make it a bit easier.

## How to use
Open the ARKit Curve Mapper window (under window).
Write down what the ARKit curve should map to. If you want to use one ARKit curve to drive multiple curves separate with `,`. You then save the file using the export button (this is a csv file so in here the multiple curves are separated with `|` instead). Drag the csv into unreal engine, make sure to set the rows to `ARKitMappingRow`.
![alt text](Pictures\GenerateCurveTable.png)

Then open an animation blueprint's event graph for the avatar to drive the curves on. Evaluate the live link frame and feed the property names and values into the `Build Curve Map from ARKit Data Table` function. Also use the mapping table you just imported and use a local curve map (names to floats).
![alt text](Pictures\SetCurves.png)

In the animation graph, modify the curves using the curve map and your done.
![alt text](Pictures\ApplyCurves.png)