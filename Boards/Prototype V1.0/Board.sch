<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.1.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="JHNuaMao">
<description>JNHuaMao Technology Company, Professional bluetooth products suppliers. More info on http://www.jnhuamao.cn/ • Eagle Library by http://keeward.com/</description>
<packages>
<package name="26.9X13X2.2_BOARD">
<description>HM-01, HM-02, HM-09, HM-10 have same size and same pins. 26.9mm x 13mm x 2.2 mm</description>
<wire x1="-7.25" y1="7.1" x2="5.75" y2="7.1" width="0.127" layer="21" style="shortdash"/>
<wire x1="5.75" y1="7.1" x2="5.75" y2="-19.8" width="0.127" layer="21" style="shortdash"/>
<wire x1="5.75" y1="-19.8" x2="-7.25" y2="-19.8" width="0.127" layer="21" style="shortdash"/>
<wire x1="-7.25" y1="-19.8" x2="-7.25" y2="7.1" width="0.127" layer="21" style="shortdash"/>
<text x="-0.69" y="2.82" size="0.6096" layer="21" rot="R180" align="center">http://www.jnhuamao.cn/</text>
<smd name="1" x="-7.1825" y="0" dx="1.5" dy="0.635" layer="1"/>
<smd name="2" x="-7.1825" y="-1.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="3" x="-7.1825" y="-3" dx="1.5" dy="0.635" layer="1"/>
<smd name="4" x="-7.1825" y="-4.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="5" x="-7.1825" y="-6" dx="1.5" dy="0.635" layer="1"/>
<smd name="6" x="-7.1825" y="-7.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="$7" x="-7.1825" y="-9" dx="1.5" dy="0.635" layer="1"/>
<smd name="8" x="-7.1825" y="-10.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="9" x="-7.1825" y="-12" dx="1.5" dy="0.635" layer="1"/>
<smd name="10" x="-7.1825" y="-13.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="11" x="-7.1825" y="-15" dx="1.5" dy="0.635" layer="1"/>
<smd name="12" x="-7.1825" y="-16.5" dx="1.5" dy="0.635" layer="1"/>
<smd name="13" x="-7.1825" y="-18" dx="1.5" dy="0.635" layer="1"/>
<smd name="14" x="-6" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="15" x="-4.5" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="16" x="-3" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="17" x="-1.5" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="18" x="0" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="19" x="1.5" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="20" x="3" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="21" x="4.5" y="-19.5575" dx="1.5" dy="0.635" layer="1" rot="R90"/>
<smd name="22" x="5.6825" y="-18" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="23" x="5.6825" y="-16.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="24" x="5.6825" y="-15" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="25" x="5.6825" y="-13.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="26" x="5.6825" y="-12" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="27" x="5.6825" y="-10.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="28" x="5.6825" y="-9" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="29" x="5.6825" y="-7.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="30" x="5.6825" y="-6" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="31" x="5.6825" y="-4.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="32" x="5.6825" y="-3" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<smd name="33" x="5.6825" y="-1.5" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<text x="-6" y="0" size="0.8128" layer="21" align="center-left">P1_6</text>
<text x="-6" y="-1.5" size="0.8128" layer="21" align="center-left">P1_7</text>
<text x="-6" y="-3" size="0.8128" layer="21" align="center-left">P1_4</text>
<text x="-6" y="-4.5" size="0.8128" layer="21" align="center-left">P1_5</text>
<text x="-6" y="-15" size="0.8128" layer="21" align="center-left">RST</text>
<text x="-6" y="-16.5" size="0.8128" layer="21" align="center-left">+3.3V</text>
<text x="-6" y="-18.8075" size="0.6096" layer="21" rot="R105" align="center-left">GND</text>
<text x="-4.5" y="-18.75" size="0.8128" layer="21" rot="R90" align="center-left">D-</text>
<text x="3" y="-18.75" size="0.8128" layer="21" rot="R90" align="center-left">D+</text>
<text x="4.5" y="-18.8075" size="0.6096" layer="21" rot="R75" align="center-left">GND</text>
<text x="4.5" y="-16.5" size="0.8128" layer="21" distance="48" align="center-right">P1_3</text>
<text x="4.5" y="-15" size="0.8128" layer="21" distance="48" align="center-right">P1_2</text>
<text x="4.5" y="-13.5" size="0.8128" layer="21" distance="48" align="center-right">P1_1</text>
<text x="4.5" y="-12" size="0.8128" layer="21" distance="48" align="center-right">P1_0</text>
<text x="4.5" y="-10.5" size="0.8128" layer="21" distance="48" align="center-right">P0_7</text>
<text x="4.5" y="-9" size="0.8128" layer="21" distance="48" align="center-right">P0_6</text>
<text x="4.5" y="-7.5" size="0.8128" layer="21" distance="48" align="center-right">P0_5</text>
<text x="4.5" y="-6" size="0.8128" layer="21" distance="48" align="center-right">P0_4</text>
<text x="4.5" y="-4.5" size="0.8128" layer="21" distance="48" align="center-right">P0_3</text>
<text x="4.5" y="-3" size="0.8128" layer="21" distance="48" align="center-right">P0_2</text>
<text x="4.5" y="-1.5" size="0.8128" layer="21" distance="48" align="center-right">P0_1</text>
<smd name="34" x="5.6825" y="0" dx="1.5" dy="0.635" layer="1" rot="R180"/>
<text x="4.5" y="0" size="0.8128" layer="21" distance="48" align="center-right">P0_0</text>
<text x="-0.69" y="4.32" size="1.778" layer="21" align="center">HM-10</text>
<text x="-6" y="-8.945" size="0.8128" layer="21" align="center-left">P2_2</text>
<text x="-6" y="-12.12" size="0.8128" layer="21" align="center-left">P2_0</text>
<text x="-6" y="-10.469" size="0.8128" layer="21" align="center-left">P2_1</text>
</package>
</packages>
<symbols>
<symbol name="HM-10">
<description>More informations on http://www.jnhuamao.cn/Bluetooth40_en.zip</description>
<text x="5.54" y="-8.08" size="3.81" layer="94">HM-10</text>
<text x="1.27" y="-1.19" size="1.4224" layer="94" align="top-left">http://www.jnhuamao.cn/</text>
<pin name="P1_6" x="-5.08" y="-10.16" length="middle"/>
<pin name="P1_7" x="-5.08" y="-12.7" length="middle"/>
<pin name="P1_4" x="-5.08" y="-15.24" length="middle"/>
<pin name="P1_5" x="-5.08" y="-17.78" length="middle"/>
<pin name="NC5" x="-5.08" y="-20.32" length="middle"/>
<pin name="NC6" x="-5.08" y="-22.86" length="middle"/>
<pin name="P2_2/DC" x="-5.08" y="-25.4" length="middle"/>
<pin name="P2_1/DD" x="-5.08" y="-27.94" length="middle"/>
<pin name="P2_0" x="-5.08" y="-30.48" length="middle"/>
<pin name="NC10" x="-5.08" y="-33.02" length="middle"/>
<pin name="RESET" x="-5.08" y="-35.56" length="middle"/>
<pin name="+3.3V" x="-5.08" y="-38.1" length="middle"/>
<pin name="GND" x="-5.08" y="-40.64" length="middle"/>
<pin name="USB_D-" x="6.35" y="-60.96" length="middle" rot="R90"/>
<pin name="NC16" x="8.89" y="-60.96" length="middle" rot="R90"/>
<pin name="NC17" x="11.43" y="-60.96" length="middle" rot="R90"/>
<pin name="NC18" x="13.97" y="-60.96" length="middle" rot="R90"/>
<pin name="NC19" x="16.51" y="-60.96" length="middle" rot="R90"/>
<pin name="USB_D+" x="19.05" y="-60.96" length="middle" rot="R90"/>
<wire x1="0" y1="0" x2="25.4" y2="0" width="0.254" layer="94"/>
<wire x1="25.4" y1="0" x2="25.4" y2="-55.88" width="0.254" layer="94"/>
<wire x1="25.4" y1="-55.88" x2="0" y2="-55.88" width="0.254" layer="94"/>
<wire x1="0" y1="-55.88" x2="0" y2="0" width="0.254" layer="94"/>
<pin name="P1_3" x="30.48" y="-38.1" length="middle" rot="R180"/>
<pin name="P1_2" x="30.48" y="-35.56" length="middle" rot="R180"/>
<pin name="P1_1" x="30.48" y="-33.02" length="middle" rot="R180"/>
<pin name="P1_0" x="30.48" y="-30.48" length="middle" rot="R180"/>
<pin name="P0_7" x="30.48" y="-27.94" length="middle" rot="R180"/>
<pin name="P0_6" x="30.48" y="-25.4" length="middle" rot="R180"/>
<pin name="P0_5" x="30.48" y="-22.86" length="middle" rot="R180"/>
<pin name="P0_4" x="30.48" y="-20.32" length="middle" rot="R180"/>
<pin name="P0_3" x="30.48" y="-17.78" length="middle" rot="R180"/>
<pin name="P0_2" x="30.48" y="-15.24" length="middle" rot="R180"/>
<pin name="P0_1" x="30.48" y="-12.7" length="middle" rot="R180"/>
<pin name="P0_0" x="30.48" y="-10.16" length="middle" rot="R180"/>
<text x="15.24" y="-45.72" size="1.6764" layer="94" rot="R90">&gt;NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="HM-10" prefix="E">
<description>HM Bluetooth module use TI CC2540 or CC2541, Master and slave roles in one, transmission version and remote control version and PIO state acquisition functions in one, Support the AT command modify module parameters, Convenient and flexible.</description>
<gates>
<gate name="G$1" symbol="HM-10" x="17.78" y="-17.78"/>
</gates>
<devices>
<device name="-BOARD" package="26.9X13X2.2_BOARD">
<connects>
<connect gate="G$1" pin="+3.3V" pad="12"/>
<connect gate="G$1" pin="GND" pad="13 14 21 22"/>
<connect gate="G$1" pin="NC10" pad="10"/>
<connect gate="G$1" pin="NC16" pad="16"/>
<connect gate="G$1" pin="NC17" pad="17"/>
<connect gate="G$1" pin="NC18" pad="18"/>
<connect gate="G$1" pin="NC19" pad="19"/>
<connect gate="G$1" pin="NC5" pad="5"/>
<connect gate="G$1" pin="NC6" pad="6"/>
<connect gate="G$1" pin="P0_0" pad="34"/>
<connect gate="G$1" pin="P0_1" pad="33"/>
<connect gate="G$1" pin="P0_2" pad="32"/>
<connect gate="G$1" pin="P0_3" pad="31"/>
<connect gate="G$1" pin="P0_4" pad="30"/>
<connect gate="G$1" pin="P0_5" pad="29"/>
<connect gate="G$1" pin="P0_6" pad="28"/>
<connect gate="G$1" pin="P0_7" pad="27"/>
<connect gate="G$1" pin="P1_0" pad="26"/>
<connect gate="G$1" pin="P1_1" pad="25"/>
<connect gate="G$1" pin="P1_2" pad="24"/>
<connect gate="G$1" pin="P1_3" pad="23"/>
<connect gate="G$1" pin="P1_4" pad="3"/>
<connect gate="G$1" pin="P1_5" pad="4"/>
<connect gate="G$1" pin="P1_6" pad="1"/>
<connect gate="G$1" pin="P1_7" pad="2"/>
<connect gate="G$1" pin="P2_0" pad="9"/>
<connect gate="G$1" pin="P2_1/DD" pad="8"/>
<connect gate="G$1" pin="P2_2/DC" pad="$7"/>
<connect gate="G$1" pin="RESET" pad="11"/>
<connect gate="G$1" pin="USB_D+" pad="20"/>
<connect gate="G$1" pin="USB_D-" pad="15"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="wirepad">
<description>&lt;b&gt;Single Pads&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="SMD1,27-2,54">
<description>&lt;b&gt;SMD PAD&lt;/b&gt;</description>
<smd name="1" x="0" y="0" dx="1.27" dy="2.54" layer="1"/>
<text x="0" y="0" size="0.0254" layer="27">&gt;VALUE</text>
<text x="-0.8" y="-2.4" size="1.27" layer="25" rot="R90">&gt;NAME</text>
</package>
<package name="1,6/0,9">
<description>&lt;b&gt;THROUGH-HOLE PAD&lt;/b&gt;</description>
<wire x1="-0.508" y1="0.762" x2="-0.762" y2="0.762" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="0.762" x2="-0.762" y2="0.508" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-0.508" x2="-0.762" y2="-0.762" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-0.762" x2="-0.508" y2="-0.762" width="0.1524" layer="21"/>
<wire x1="0.508" y1="-0.762" x2="0.762" y2="-0.762" width="0.1524" layer="21"/>
<wire x1="0.762" y1="-0.762" x2="0.762" y2="-0.508" width="0.1524" layer="21"/>
<wire x1="0.762" y1="0.508" x2="0.762" y2="0.762" width="0.1524" layer="21"/>
<wire x1="0.762" y1="0.762" x2="0.508" y2="0.762" width="0.1524" layer="21"/>
<circle x="0" y="0" radius="0.635" width="0.1524" layer="51"/>
<pad name="1" x="0" y="0" drill="0.9144" diameter="1.6002" shape="octagon"/>
<text x="-0.762" y="1.016" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="0" y="0.6" size="0.0254" layer="27">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="PAD">
<wire x1="-1.016" y1="1.016" x2="1.016" y2="-1.016" width="0.254" layer="94"/>
<wire x1="-1.016" y1="-1.016" x2="1.016" y2="1.016" width="0.254" layer="94"/>
<text x="-1.143" y="1.8542" size="1.778" layer="95">&gt;NAME</text>
<text x="-1.143" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
<pin name="P" x="2.54" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="SMD2" prefix="PAD" uservalue="yes">
<description>&lt;b&gt;SMD PAD&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="PAD" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMD1,27-2,54">
<connects>
<connect gate="1" pin="P" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="1,6/0,9" prefix="PAD" uservalue="yes">
<description>&lt;b&gt;THROUGH-HOLE PAD&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="PAD" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1,6/0,9">
<connects>
<connect gate="1" pin="P" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply2">
<description>&lt;b&gt;Supply Symbols&lt;/b&gt;&lt;p&gt;
GND, VCC, 0V, +5V, -5V, etc.&lt;p&gt;
Please keep in mind, that these devices are necessary for the
automatic wiring of the supply signals.&lt;p&gt;
The pin name defined in the symbol is identical to the net which is to be wired automatically.&lt;p&gt;
In this library the device names are the same as the pin names of the symbols, therefore the correct signal names appear next to the supply symbols in the schematic.&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="GND">
<wire x1="-1.27" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<text x="-1.905" y="-3.175" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
<symbol name="VDD">
<circle x="0" y="1.27" radius="1.27" width="0.254" layer="94"/>
<text x="-1.905" y="3.175" size="1.778" layer="96">&gt;VALUE</text>
<pin name="VDD" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="SUPPLY">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="GND" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="VDD" prefix="SUPPLY">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="VDD" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="E1" library="JHNuaMao" deviceset="HM-10" device="-BOARD"/>
<part name="+3.3V" library="wirepad" deviceset="SMD2" device=""/>
<part name="RESET" library="wirepad" deviceset="SMD2" device=""/>
<part name="DC" library="wirepad" deviceset="SMD2" device=""/>
<part name="DD" library="wirepad" deviceset="SMD2" device=""/>
<part name="GND" library="wirepad" deviceset="SMD2" device=""/>
<part name="P0_0" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_2" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_3" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_4" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_5" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="VDD" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="GND1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD2" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD3" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD4" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD5" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD6" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD7" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD8" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD9" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD10" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD11" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD12" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_6" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P0_7" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_0" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_2" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_3" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_4" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_5" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_6" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P1_7" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P2_0" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P2_1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="P2_2" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD15" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD16" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD17" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD18" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD19" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD20" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD21" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD22" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD23" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD24" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD25" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD26" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD27" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD28" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD29" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD30" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD31" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD32" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD33" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD34" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD35" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD36" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD37" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD38" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD39" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD40" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD41" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD42" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD43" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD44" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD45" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD46" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="PAD47" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="GND2" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="VDD1" library="wirepad" deviceset="1,6/0,9" device=""/>
<part name="SUPPLY1" library="supply2" deviceset="GND" device=""/>
<part name="SUPPLY2" library="supply2" deviceset="VDD" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="E1" gate="G$1" x="35.56" y="116.84"/>
<instance part="+3.3V" gate="1" x="-20.32" y="99.06"/>
<instance part="RESET" gate="1" x="-20.32" y="81.28"/>
<instance part="DC" gate="1" x="-20.32" y="91.44"/>
<instance part="DD" gate="1" x="-20.32" y="88.9"/>
<instance part="GND" gate="1" x="-20.32" y="76.2"/>
<instance part="P0_0" gate="1" x="132.08" y="106.68" rot="MR0"/>
<instance part="P0_1" gate="1" x="132.08" y="104.14" rot="MR0"/>
<instance part="P0_2" gate="1" x="132.08" y="101.6" rot="MR0"/>
<instance part="P0_3" gate="1" x="132.08" y="99.06" rot="MR0"/>
<instance part="P0_4" gate="1" x="132.08" y="96.52" rot="MR0"/>
<instance part="P0_5" gate="1" x="132.08" y="93.98" rot="MR0"/>
<instance part="VDD" gate="1" x="-7.62" y="109.22"/>
<instance part="GND1" gate="1" x="-7.62" y="68.58"/>
<instance part="PAD1" gate="1" x="-2.54" y="22.86"/>
<instance part="PAD2" gate="1" x="17.78" y="22.86"/>
<instance part="PAD3" gate="1" x="38.1" y="22.86"/>
<instance part="PAD4" gate="1" x="53.34" y="22.86"/>
<instance part="PAD5" gate="1" x="66.04" y="22.86"/>
<instance part="PAD6" gate="1" x="81.28" y="22.86"/>
<instance part="PAD7" gate="1" x="-2.54" y="17.78"/>
<instance part="PAD8" gate="1" x="17.78" y="17.78"/>
<instance part="PAD9" gate="1" x="38.1" y="17.78"/>
<instance part="PAD10" gate="1" x="53.34" y="17.78"/>
<instance part="PAD11" gate="1" x="66.04" y="17.78"/>
<instance part="PAD12" gate="1" x="81.28" y="17.78"/>
<instance part="P0_6" gate="1" x="132.08" y="91.44" rot="MR0"/>
<instance part="P0_7" gate="1" x="132.08" y="88.9" rot="MR0"/>
<instance part="P1_0" gate="1" x="132.08" y="86.36" rot="MR0"/>
<instance part="P1_1" gate="1" x="132.08" y="83.82" rot="MR0"/>
<instance part="P1_2" gate="1" x="132.08" y="81.28" rot="MR0"/>
<instance part="P1_3" gate="1" x="132.08" y="78.74" rot="MR0"/>
<instance part="P1_4" gate="1" x="53.34" y="139.7" rot="MR0"/>
<instance part="P1_5" gate="1" x="53.34" y="137.16" rot="MR0"/>
<instance part="P1_6" gate="1" x="53.34" y="134.62" rot="MR0"/>
<instance part="P1_7" gate="1" x="53.34" y="132.08" rot="MR0"/>
<instance part="P2_0" gate="1" x="30.48" y="48.26" rot="MR0"/>
<instance part="P2_1" gate="1" x="0" y="43.18"/>
<instance part="P2_2" gate="1" x="0" y="50.8"/>
<instance part="PAD15" gate="1" x="38.1" y="10.16"/>
<instance part="PAD16" gate="1" x="53.34" y="10.16"/>
<instance part="PAD17" gate="1" x="66.04" y="10.16"/>
<instance part="PAD18" gate="1" x="81.28" y="10.16"/>
<instance part="PAD19" gate="1" x="-2.54" y="5.08"/>
<instance part="PAD20" gate="1" x="17.78" y="5.08"/>
<instance part="PAD21" gate="1" x="38.1" y="5.08"/>
<instance part="PAD22" gate="1" x="53.34" y="5.08"/>
<instance part="PAD23" gate="1" x="66.04" y="5.08"/>
<instance part="PAD24" gate="1" x="81.28" y="5.08"/>
<instance part="PAD25" gate="1" x="-7.62" y="-2.54"/>
<instance part="PAD26" gate="1" x="12.7" y="-2.54"/>
<instance part="PAD27" gate="1" x="33.02" y="-2.54"/>
<instance part="PAD28" gate="1" x="48.26" y="-2.54"/>
<instance part="PAD29" gate="1" x="60.96" y="-2.54"/>
<instance part="PAD30" gate="1" x="76.2" y="-2.54"/>
<instance part="PAD31" gate="1" x="-7.62" y="-7.62"/>
<instance part="PAD32" gate="1" x="12.7" y="-7.62"/>
<instance part="PAD33" gate="1" x="33.02" y="-7.62"/>
<instance part="PAD34" gate="1" x="48.26" y="-7.62"/>
<instance part="PAD35" gate="1" x="60.96" y="-7.62"/>
<instance part="PAD36" gate="1" x="76.2" y="-7.62"/>
<instance part="PAD37" gate="1" x="-7.62" y="-15.24"/>
<instance part="PAD38" gate="1" x="12.7" y="-15.24"/>
<instance part="PAD39" gate="1" x="33.02" y="-15.24"/>
<instance part="PAD40" gate="1" x="48.26" y="-15.24"/>
<instance part="PAD41" gate="1" x="60.96" y="-15.24"/>
<instance part="PAD42" gate="1" x="76.2" y="-15.24"/>
<instance part="PAD43" gate="1" x="-7.62" y="-20.32"/>
<instance part="PAD44" gate="1" x="12.7" y="-20.32"/>
<instance part="PAD45" gate="1" x="33.02" y="-20.32"/>
<instance part="PAD46" gate="1" x="48.26" y="-20.32"/>
<instance part="PAD47" gate="1" x="60.96" y="-20.32"/>
<instance part="GND2" gate="1" x="-7.62" y="63.5"/>
<instance part="VDD1" gate="1" x="-7.62" y="114.3"/>
<instance part="SUPPLY1" gate="GND" x="15.24" y="60.96"/>
<instance part="SUPPLY2" gate="G$1" x="15.24" y="116.84"/>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<pinref part="GND" gate="1" pin="P"/>
<wire x1="-17.78" y1="76.2" x2="15.24" y2="76.2" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="GND"/>
<wire x1="15.24" y1="76.2" x2="30.48" y2="76.2" width="0.1524" layer="91"/>
<pinref part="GND1" gate="1" pin="P"/>
<wire x1="-5.08" y1="68.58" x2="15.24" y2="68.58" width="0.1524" layer="91"/>
<wire x1="15.24" y1="68.58" x2="15.24" y2="76.2" width="0.1524" layer="91"/>
<junction x="15.24" y="76.2"/>
<pinref part="GND2" gate="1" pin="P"/>
<wire x1="-5.08" y1="63.5" x2="15.24" y2="63.5" width="0.1524" layer="91"/>
<wire x1="15.24" y1="63.5" x2="15.24" y2="68.58" width="0.1524" layer="91"/>
<junction x="15.24" y="68.58"/>
<pinref part="SUPPLY1" gate="GND" pin="GND"/>
<junction x="15.24" y="63.5"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="RESET" gate="1" pin="P"/>
<wire x1="-17.78" y1="81.28" x2="30.48" y2="81.28" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="RESET"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="DC" gate="1" pin="P"/>
<wire x1="-17.78" y1="91.44" x2="30.48" y2="91.44" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P2_2/DC"/>
</segment>
</net>
<net name="N$5" class="0">
<segment>
<pinref part="DD" gate="1" pin="P"/>
<wire x1="-17.78" y1="88.9" x2="30.48" y2="88.9" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P2_1/DD"/>
</segment>
</net>
<net name="P0_0" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_0"/>
<pinref part="P0_0" gate="1" pin="P"/>
<wire x1="66.04" y1="106.68" x2="129.54" y2="106.68" width="0.1524" layer="91"/>
</segment>
</net>
<net name="P0_1" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_1"/>
<pinref part="P0_1" gate="1" pin="P"/>
<wire x1="66.04" y1="104.14" x2="129.54" y2="104.14" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_3"/>
<wire x1="66.04" y1="99.06" x2="129.54" y2="99.06" width="0.1524" layer="91"/>
<pinref part="P0_3" gate="1" pin="P"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_4"/>
<wire x1="66.04" y1="96.52" x2="129.54" y2="96.52" width="0.1524" layer="91"/>
<pinref part="P0_4" gate="1" pin="P"/>
</segment>
</net>
<net name="N$1" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_6"/>
<wire x1="66.04" y1="91.44" x2="129.54" y2="91.44" width="0.1524" layer="91"/>
<pinref part="P0_6" gate="1" pin="P"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_7"/>
<wire x1="66.04" y1="88.9" x2="129.54" y2="88.9" width="0.1524" layer="91"/>
<pinref part="P0_7" gate="1" pin="P"/>
</segment>
</net>
<net name="N$7" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P1_2"/>
<pinref part="P1_2" gate="1" pin="P"/>
<wire x1="66.04" y1="81.28" x2="129.54" y2="81.28" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P1_1"/>
<pinref part="P1_1" gate="1" pin="P"/>
<wire x1="66.04" y1="83.82" x2="129.54" y2="83.82" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<pinref part="P1_7" gate="1" pin="P"/>
<wire x1="50.8" y1="132.08" x2="22.86" y2="132.08" width="0.1524" layer="91"/>
<wire x1="22.86" y1="132.08" x2="22.86" y2="104.14" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P1_7"/>
<wire x1="22.86" y1="104.14" x2="30.48" y2="104.14" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<pinref part="P1_6" gate="1" pin="P"/>
<wire x1="50.8" y1="134.62" x2="25.4" y2="134.62" width="0.1524" layer="91"/>
<wire x1="25.4" y1="134.62" x2="25.4" y2="106.68" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P1_6"/>
<wire x1="25.4" y1="106.68" x2="30.48" y2="106.68" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<pinref part="P1_5" gate="1" pin="P"/>
<wire x1="50.8" y1="137.16" x2="20.32" y2="137.16" width="0.1524" layer="91"/>
<wire x1="20.32" y1="137.16" x2="20.32" y2="99.06" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P1_5"/>
<wire x1="20.32" y1="99.06" x2="30.48" y2="99.06" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$16" class="0">
<segment>
<pinref part="P1_4" gate="1" pin="P"/>
<wire x1="50.8" y1="139.7" x2="27.94" y2="139.7" width="0.1524" layer="91"/>
<wire x1="27.94" y1="139.7" x2="27.94" y2="101.6" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="P1_4"/>
<wire x1="27.94" y1="101.6" x2="30.48" y2="101.6" width="0.1524" layer="91"/>
</segment>
</net>
<net name="VDD" class="0">
<segment>
<pinref part="+3.3V" gate="1" pin="P"/>
<wire x1="-17.78" y1="99.06" x2="15.24" y2="99.06" width="0.1524" layer="91"/>
<wire x1="15.24" y1="99.06" x2="15.24" y2="78.74" width="0.1524" layer="91"/>
<pinref part="E1" gate="G$1" pin="+3.3V"/>
<wire x1="15.24" y1="78.74" x2="30.48" y2="78.74" width="0.1524" layer="91"/>
<pinref part="VDD" gate="1" pin="P"/>
<wire x1="-5.08" y1="109.22" x2="15.24" y2="109.22" width="0.1524" layer="91"/>
<wire x1="15.24" y1="109.22" x2="15.24" y2="99.06" width="0.1524" layer="91"/>
<junction x="15.24" y="99.06"/>
<pinref part="VDD1" gate="1" pin="P"/>
<wire x1="-5.08" y1="114.3" x2="15.24" y2="114.3" width="0.1524" layer="91"/>
<wire x1="15.24" y1="114.3" x2="15.24" y2="109.22" width="0.1524" layer="91"/>
<junction x="15.24" y="109.22"/>
<pinref part="SUPPLY2" gate="G$1" pin="VDD"/>
<junction x="15.24" y="114.3"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P1_3"/>
<pinref part="P1_3" gate="1" pin="P"/>
<wire x1="66.04" y1="78.74" x2="129.54" y2="78.74" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_5"/>
<pinref part="P0_5" gate="1" pin="P"/>
<wire x1="66.04" y1="93.98" x2="129.54" y2="93.98" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$18" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P0_2"/>
<pinref part="P0_2" gate="1" pin="P"/>
<wire x1="66.04" y1="101.6" x2="129.54" y2="101.6" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<pinref part="E1" gate="G$1" pin="P1_0"/>
<pinref part="P1_0" gate="1" pin="P"/>
<wire x1="66.04" y1="86.36" x2="129.54" y2="86.36" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
