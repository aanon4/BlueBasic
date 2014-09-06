%1\..\..\common\cc2540\cc254x_sim2bin.exe %1\%2-ImgB\Exe\BlueBasic.sim %1\..\..\..\..\..\hex\BlueBasic-%2.bin
%1\..\..\common\cc2540\srec_cat.exe %1\..\..\util\BIM\cc254x\CC254xF256\Exe\BIM_CC254xF256.hex -intel %1\..\..\BlueBasicLoader\Build\%2-ImgA\Exe\BlueBasicLoader.hex -intel %1\%2-ImgB\Exe\BlueBasic.hex -intel -o %1\..\..\..\..\..\hex\BlueBasic-%2.hex -intel
