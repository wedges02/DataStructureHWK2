**CONVERTER**
There is a cpp file on the outside called converter.cpp, together with a converter.out. The converter.out can be used to convert test_case files into a csv format that is copy pastable into desmos, and can also convert desmos table data back into the csv format.
For conversion to the desmos format, cases with multiple rings will be seperated into multiple files. To import into desmos, the entire contents of the file can be copied and directly pasted into Desmos, and it will be automatically converted into a table.
Desmos then can be used to modify said data, or to just simply visualize it to see where we may be going wrong.

To use Desmos to modify said data, first note down the header for the table (Like x1,y1). Beside the Y header, long click on the colored button and select the Drag option.
Now, each individual points can be dragged along the grid.
In a newline, write polygon(x1,y1) to visualize the entire table as a shape.
You can assign the polygon to a variable like a = polygon(x1,y1).
If you do that, you can use area(a) and perimeter(a) to find out associate information about the polygon.

To export the data from Desmos, it is a little more complicated. Right click on the page or use whichever hotkey to inspect element on the browser. Then go into the console tab.
Paste this javascript code into the console to get it to print the data in the tables:

state = Calc.getState()

for (let i = 0; i < state.expressions.list.length; i++) {
  if (state.expressions.list[i].type == "table") {
    for (let j = 0; j < state.expressions.list[i].columns.length; j++) {
      console.log(state.expressions.list[i].columns[j].latex + " = " + state.expressions.list[i].columns[j].values.toString())
    }
  }
}

The data will be seperated by x and y, i.e. x_{1} = 100,200,300,400. When running converter.out, paste each line in individually. The file will be automatically outputted into the test_case directory.
