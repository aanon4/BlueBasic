%1\..\..\common\cc2540\cc254x_sim2bin.exe %1\%3\Exe\BlueBasic.sim %1\..\..\..\..\..\hex\BlueBasic-%4.bin
%1\..\..\common\cc2540\srec_cat.exe %1\..\..\util\BIM\cc254x\CC254xF256\Exe\BIM_CC254xF256.hex -intel %1\..\..\BlueBasicLoader\Build\%2\Exe\BlueBasicLoader.hex -intel %1\%3\Exe\BlueBasic.hex -intel -o %1\..\..\..\..\..\hex\BlueBasic-%4.hex -intel
