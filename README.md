## Simple C Like Ini Reader
Simple ini reader made with Windows Api

Save/Load ini with simple calling
```cpp
IniReader ini("MyIniFile.ini");

int myValue = ini.ReadInteger("MySection", "MyValue", 0); // Load
ini.WriteInteger("MySection", "MyValue", myValue); // Save

// Or with the new syntax(it will assume that 0 is default value, so, if you have some value that isn't zero, use full function)

int myValue = ini["MySection"]["MyValue"]; // You can only read using these operators
// Writing is only available using full function. Ref: WriteInteger
```
