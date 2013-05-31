<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.4">
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
<library name="supply1">
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
<wire x1="-1.905" y1="0" x2="1.905" y2="0" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="GND">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="GND" x="0" y="0"/>
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
<library name="frames">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="DINA4_L">
<frame x1="0" y1="0" x2="264.16" y2="180.34" columns="4" rows="4" layer="94" border-left="no" border-top="no" border-right="no" border-bottom="no"/>
</symbol>
<symbol name="DOCFIELD">
<wire x1="0" y1="0" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="87.63" y2="15.24" width="0.1016" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="71.12" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="0" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="87.63" y1="5.08" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="101.6" y1="5.08" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="0" y1="15.24" x2="0" y2="22.86" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="22.86" x2="101.6" y2="15.24" width="0.1016" layer="94"/>
<text x="1.27" y="1.27" size="2.54" layer="94" font="vector">Date:</text>
<text x="12.7" y="1.27" size="2.54" layer="94" font="vector">&gt;LAST_DATE_TIME</text>
<text x="72.39" y="1.27" size="2.54" layer="94" font="vector">Sheet:</text>
<text x="86.36" y="1.27" size="2.54" layer="94" font="vector">&gt;SHEET</text>
<text x="88.9" y="11.43" size="2.54" layer="94" font="vector">REV:</text>
<text x="1.27" y="19.05" size="2.54" layer="94" font="vector">TITLE:</text>
<text x="1.27" y="11.43" size="2.54" layer="94" font="vector">Document Number:</text>
<text x="17.78" y="19.05" size="2.54" layer="94" font="vector">&gt;DRAWING_NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="DINA4_L" prefix="FRAME" uservalue="yes">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A4, landscape with extra doc field</description>
<gates>
<gate name="G$1" symbol="DINA4_L" x="0" y="0"/>
<gate name="G$2" symbol="DOCFIELD" x="162.56" y="0" addlevel="must"/>
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
<library name="con-subd">
<description>&lt;b&gt;SUB-D Connectors&lt;/b&gt;&lt;p&gt;
Harting&lt;br&gt;
NorComp&lt;br&gt;
&lt;p&gt;
PREFIX :&lt;br&gt;
H = High density&lt;br&gt;
F = Female&lt;br&gt;
M = Male&lt;p&gt;
NUMBER: Number of pins&lt;p&gt;
SUFFIX :&lt;br&gt;
H = Horizontal&lt;br&gt;
V = Vertical &lt;br&gt;
P = Shield pin on housing&lt;br&gt;
B = No mounting holes&lt;br&gt;
S = Pins individually placeable (Single)&lt;br&gt;
D = Direct mounting &lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="M25HP">
<description>&lt;b&gt;SUB-D&lt;/b&gt;</description>
<wire x1="19.304" y1="-17.526" x2="19.05" y2="-17.526" width="0.1524" layer="21"/>
<wire x1="19.304" y1="-17.526" x2="19.812" y2="-17.018" width="0.1524" layer="21" curve="90"/>
<wire x1="-19.812" y1="-17.018" x2="-19.304" y2="-17.526" width="0.1524" layer="21" curve="90"/>
<wire x1="19.431" y1="-11.684" x2="19.431" y2="-17.018" width="0.1524" layer="21"/>
<wire x1="19.431" y1="-11.684" x2="-19.431" y2="-11.684" width="0.1524" layer="21"/>
<wire x1="-19.431" y1="-11.684" x2="-19.431" y2="-17.018" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-7.62" x2="21.463" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-7.62" x2="26.543" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-7.62" x2="26.543" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-10.668" x2="24.003" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-10.668" x2="26.543" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-11.176" x2="26.543" y2="-11.684" width="0.1524" layer="21"/>
<wire x1="-26.543" y1="-11.684" x2="-26.543" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-26.543" y1="-11.176" x2="-26.543" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="-26.543" y1="-10.668" x2="-26.543" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-26.543" y1="-7.62" x2="-26.543" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="24.003" y1="-10.414" x2="21.463" y2="-10.414" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-11.176" x2="24.003" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="24.003" y1="-11.176" x2="21.463" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="24.003" y1="-10.668" x2="24.003" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="24.003" y1="-10.668" x2="24.003" y2="-10.414" width="0.1524" layer="21"/>
<wire x1="21.463" y1="-10.414" x2="21.463" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="21.463" y1="-10.668" x2="21.463" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="26.543" y1="-7.493" x2="21.463" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="21.463" y1="-7.493" x2="21.463" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="21.463" y1="-10.668" x2="21.336" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="21.336" y1="-10.668" x2="-21.336" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="20.701" y1="-7.62" x2="21.336" y2="-8.255" width="0.1524" layer="21" curve="-90"/>
<wire x1="21.463" y1="-7.62" x2="20.574" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="20.574" y1="-7.62" x2="20.32" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="21.336" y1="-8.255" x2="21.336" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="20.32" y1="-7.62" x2="20.32" y2="-6.858" width="0.1524" layer="21"/>
<wire x1="20.32" y1="-7.62" x2="-20.32" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="20.32" y1="-6.858" x2="-20.32" y2="-6.858" width="0.1524" layer="21"/>
<wire x1="-20.32" y1="-7.62" x2="-20.32" y2="-6.858" width="0.1524" layer="21"/>
<wire x1="-20.32" y1="-7.62" x2="-20.574" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-20.574" y1="-7.62" x2="-21.463" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-7.493" x2="-26.543" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="-24.003" y1="-10.668" x2="-24.003" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-24.003" y1="-10.668" x2="-26.543" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="-24.003" y1="-11.176" x2="-26.543" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-10.414" x2="-24.003" y2="-10.414" width="0.1524" layer="21"/>
<wire x1="-24.003" y1="-10.668" x2="-24.003" y2="-10.414" width="0.1524" layer="21"/>
<wire x1="21.463" y1="-11.176" x2="-21.463" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-11.176" x2="-24.003" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-10.414" x2="-21.463" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-10.668" x2="-21.463" y2="-11.176" width="0.1524" layer="21"/>
<wire x1="-21.336" y1="-8.255" x2="-21.336" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="-21.336" y1="-10.668" x2="-21.463" y2="-10.668" width="0.1524" layer="21"/>
<wire x1="-21.336" y1="-8.255" x2="-20.701" y2="-7.62" width="0.1524" layer="21" curve="-90"/>
<wire x1="25.908" y1="3.175" x2="26.543" y2="2.54" width="0.1524" layer="21"/>
<wire x1="26.543" y1="2.54" x2="26.543" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="21.082" y1="3.175" x2="25.908" y2="3.175" width="0.1524" layer="21"/>
<wire x1="21.082" y1="3.175" x2="20.574" y2="2.667" width="0.1524" layer="21"/>
<wire x1="20.574" y1="2.667" x2="20.574" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-20.574" y1="2.667" x2="-20.574" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-20.574" y1="2.667" x2="-21.082" y2="3.175" width="0.1524" layer="21"/>
<wire x1="-21.082" y1="3.175" x2="-26.035" y2="3.175" width="0.1524" layer="21"/>
<wire x1="-26.035" y1="3.175" x2="-26.543" y2="2.667" width="0.1524" layer="21"/>
<wire x1="-26.543" y1="2.667" x2="-26.543" y2="-7.493" width="0.1524" layer="21"/>
<wire x1="-2.7686" y1="1.143" x2="-2.7686" y2="0.127" width="0.8128" layer="51"/>
<wire x1="0" y1="1.143" x2="0" y2="0.127" width="0.8128" layer="51"/>
<wire x1="2.7686" y1="1.143" x2="2.7686" y2="0.127" width="0.8128" layer="51"/>
<wire x1="1.3716" y1="-1.397" x2="1.3716" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="-1.3716" y1="-1.397" x2="-1.3716" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="-4.1402" y1="-1.397" x2="-4.1402" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="-5.5118" y1="1.143" x2="-5.5118" y2="0.127" width="0.8128" layer="51"/>
<wire x1="-6.9088" y1="-1.397" x2="-6.9088" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="-8.2804" y1="1.143" x2="-8.2804" y2="0.127" width="0.8128" layer="51"/>
<wire x1="-5.08" y1="-13.97" x2="-2.794" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="-13.97" x2="-2.794" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="-16.256" x2="-3.175" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="-16.256" x2="-3.175" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="-14.351" x2="-4.699" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-4.699" y1="-14.351" x2="-4.699" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-4.699" y1="-16.256" x2="-5.08" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-16.256" x2="-5.08" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-1.143" y1="-13.97" x2="1.143" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="1.143" y1="-13.97" x2="1.143" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="1.143" y1="-16.256" x2="0.762" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="0.762" y1="-16.256" x2="0.762" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="0.762" y1="-14.351" x2="-0.762" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-14.351" x2="-0.762" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-16.256" x2="-1.143" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-1.143" y1="-16.256" x2="-1.143" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="2.794" y1="-13.97" x2="5.08" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-13.97" x2="5.08" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-16.256" x2="4.699" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="4.699" y1="-16.256" x2="4.699" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="4.699" y1="-14.351" x2="3.175" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="3.175" y1="-14.351" x2="3.175" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="3.175" y1="-16.256" x2="2.794" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="2.794" y1="-16.256" x2="2.794" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-7.493" x2="-21.463" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-21.463" y1="-7.62" x2="-26.543" y2="-7.62" width="0.1524" layer="21"/>
<wire x1="-19.431" y1="-11.684" x2="-26.543" y2="-11.684" width="0.1524" layer="21"/>
<wire x1="-19.812" y1="-12.319" x2="-19.812" y2="-17.018" width="0.1524" layer="21"/>
<wire x1="-20.447" y1="-11.684" x2="-19.812" y2="-12.319" width="0.1524" layer="21" curve="-90"/>
<wire x1="26.543" y1="-11.684" x2="19.431" y2="-11.684" width="0.1524" layer="21"/>
<wire x1="19.812" y1="-12.319" x2="19.812" y2="-17.018" width="0.1524" layer="21"/>
<wire x1="19.812" y1="-12.319" x2="20.447" y2="-11.684" width="0.1524" layer="21" curve="-90"/>
<wire x1="-9.652" y1="-1.3716" x2="-9.652" y2="-2.3876" width="0.8128" layer="51"/>
<wire x1="-11.049" y1="1.143" x2="-11.049" y2="0.127" width="0.8128" layer="51"/>
<wire x1="4.1402" y1="-1.397" x2="4.1402" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="5.5118" y1="1.143" x2="5.5118" y2="0.127" width="0.8128" layer="51"/>
<wire x1="6.9088" y1="-1.397" x2="6.9088" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="8.2804" y1="1.143" x2="8.2804" y2="0.127" width="0.8128" layer="51"/>
<wire x1="-9.017" y1="-13.97" x2="-6.731" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-6.731" y1="-13.97" x2="-6.731" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-6.731" y1="-16.256" x2="-7.112" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-7.112" y1="-16.256" x2="-7.112" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-7.112" y1="-14.351" x2="-8.636" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-8.636" y1="-14.351" x2="-8.636" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-8.636" y1="-16.256" x2="-9.017" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-9.017" y1="-16.256" x2="-9.017" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="6.731" y1="-13.97" x2="9.017" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="9.017" y1="-13.97" x2="9.017" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="9.017" y1="-16.256" x2="8.636" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="8.636" y1="-16.256" x2="8.636" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="8.636" y1="-14.351" x2="7.112" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="7.112" y1="-14.351" x2="7.112" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="7.112" y1="-16.256" x2="6.731" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="6.731" y1="-16.256" x2="6.731" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="9.652" y1="-1.397" x2="9.652" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="11.049" y1="1.143" x2="11.049" y2="0.127" width="0.8128" layer="51"/>
<wire x1="13.7922" y1="1.143" x2="13.7922" y2="0.127" width="0.8128" layer="51"/>
<wire x1="12.4206" y1="-1.397" x2="12.4206" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="16.5608" y1="1.143" x2="16.5608" y2="0.127" width="0.8128" layer="51"/>
<wire x1="15.1892" y1="-1.397" x2="15.1892" y2="-2.413" width="0.8128" layer="51"/>
<wire x1="-13.7922" y1="1.143" x2="-13.7922" y2="0.127" width="0.8128" layer="51"/>
<wire x1="-12.4206" y1="-1.3716" x2="-12.4206" y2="-2.3876" width="0.8128" layer="51"/>
<wire x1="-15.1892" y1="-1.3716" x2="-15.1892" y2="-2.3876" width="0.8128" layer="51"/>
<wire x1="-16.5608" y1="1.143" x2="-16.5608" y2="0.127" width="0.8128" layer="51"/>
<wire x1="-12.954" y1="-13.97" x2="-10.668" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-10.668" y1="-13.97" x2="-10.668" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-10.668" y1="-16.256" x2="-11.049" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-11.049" y1="-16.256" x2="-11.049" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-11.049" y1="-14.351" x2="-12.573" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="-12.573" y1="-14.351" x2="-12.573" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-12.573" y1="-16.256" x2="-12.954" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="-12.954" y1="-16.256" x2="-12.954" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="10.668" y1="-13.97" x2="12.954" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="12.954" y1="-13.97" x2="12.954" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="12.954" y1="-16.256" x2="12.573" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="12.573" y1="-16.256" x2="12.573" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="12.573" y1="-14.351" x2="11.049" y2="-14.351" width="0.1524" layer="21"/>
<wire x1="11.049" y1="-14.351" x2="11.049" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="11.049" y1="-16.256" x2="10.668" y2="-16.256" width="0.1524" layer="21"/>
<wire x1="10.668" y1="-16.256" x2="10.668" y2="-13.97" width="0.1524" layer="21"/>
<wire x1="-19.05" y1="-12.192" x2="-19.05" y2="-17.526" width="0.1524" layer="21"/>
<wire x1="-19.05" y1="-17.526" x2="-19.304" y2="-17.526" width="0.1524" layer="21"/>
<wire x1="19.05" y1="-12.192" x2="19.05" y2="-17.526" width="0.1524" layer="21"/>
<wire x1="19.05" y1="-17.526" x2="-19.05" y2="-17.526" width="0.1524" layer="21"/>
<circle x="23.5204" y="0" radius="1.651" width="0.1524" layer="21"/>
<circle x="-23.5204" y="0" radius="1.651" width="0.1524" layer="21"/>
<pad name="13" x="16.5608" y="1.27" drill="1.016" shape="octagon"/>
<pad name="12" x="13.7922" y="1.27" drill="1.016" shape="octagon"/>
<pad name="11" x="11.049" y="1.27" drill="1.016" shape="octagon"/>
<pad name="10" x="8.2804" y="1.27" drill="1.016" shape="octagon"/>
<pad name="9" x="5.5118" y="1.27" drill="1.016" shape="octagon"/>
<pad name="8" x="2.7686" y="1.27" drill="1.016" shape="octagon"/>
<pad name="7" x="0" y="1.27" drill="1.016" shape="octagon"/>
<pad name="6" x="-2.7686" y="1.27" drill="1.016" shape="octagon"/>
<pad name="5" x="-5.5118" y="1.27" drill="1.016" shape="octagon"/>
<pad name="4" x="-8.2804" y="1.27" drill="1.016" shape="octagon"/>
<pad name="3" x="-11.049" y="1.27" drill="1.016" shape="octagon"/>
<pad name="2" x="-13.7922" y="1.27" drill="1.016" shape="octagon"/>
<pad name="1" x="-16.5608" y="1.27" drill="1.016" shape="octagon"/>
<pad name="25" x="15.1892" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="24" x="12.4206" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="23" x="9.652" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="22" x="6.9088" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="21" x="4.1402" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="20" x="1.3716" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="19" x="-1.3716" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="18" x="-4.1402" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="17" x="-6.9088" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="16" x="-9.652" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="15" x="-12.4206" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="14" x="-15.1892" y="-1.27" drill="1.016" shape="octagon"/>
<pad name="G1" x="-23.5204" y="0" drill="3.302" diameter="5.08"/>
<pad name="G2" x="23.5204" y="0" drill="3.302" diameter="5.08"/>
<text x="-26.416" y="4.445" size="1.778" layer="25" ratio="10">&gt;NAME</text>
<text x="-17.78" y="-20.447" size="1.778" layer="27" ratio="10">&gt;VALUE</text>
<text x="-18.669" y="0.635" size="1.27" layer="21" ratio="10">1</text>
<text x="17.907" y="0.635" size="1.27" layer="21" ratio="10">13</text>
<text x="-19.812" y="-2.032" size="1.27" layer="21" ratio="10">14</text>
<text x="17.653" y="-1.905" size="1.27" layer="21" ratio="10">25</text>
<text x="-24.638" y="-6.858" size="1.27" layer="21" ratio="10" rot="R90">2,54</text>
<text x="-18.669" y="-9.779" size="1.27" layer="21" ratio="10">M25</text>
<rectangle x1="20.701" y1="-10.668" x2="20.955" y2="-8.255" layer="21"/>
<rectangle x1="-20.955" y1="-10.668" x2="-20.701" y2="-8.255" layer="21"/>
<rectangle x1="23.0632" y1="-7.62" x2="24.0792" y2="-5.969" layer="21"/>
<rectangle x1="-24.0792" y1="-7.62" x2="-23.0632" y2="-5.969" layer="21"/>
<rectangle x1="-26.543" y1="-11.684" x2="26.543" y2="-11.176" layer="21"/>
<rectangle x1="-20.32" y1="-7.62" x2="20.32" y2="-6.858" layer="21"/>
<rectangle x1="-16.9672" y1="-6.858" x2="-16.1544" y2="0.381" layer="21"/>
<rectangle x1="-15.5956" y1="-6.858" x2="-14.7828" y2="-2.159" layer="21"/>
<rectangle x1="-14.1986" y1="-6.858" x2="-13.3858" y2="0.381" layer="21"/>
<rectangle x1="-12.827" y1="-6.858" x2="-12.0142" y2="-2.159" layer="21"/>
<rectangle x1="-11.4554" y1="-6.858" x2="-10.6426" y2="0.381" layer="21"/>
<rectangle x1="-10.0584" y1="-6.858" x2="-9.2456" y2="-2.159" layer="21"/>
<rectangle x1="-8.6868" y1="-6.858" x2="-7.874" y2="0.381" layer="21"/>
<rectangle x1="-7.3152" y1="-6.858" x2="-6.5024" y2="-2.159" layer="21"/>
<rectangle x1="-5.9182" y1="-6.858" x2="-5.1054" y2="0.381" layer="21"/>
<rectangle x1="-4.5466" y1="-6.858" x2="-3.7338" y2="-2.159" layer="21"/>
<rectangle x1="-3.175" y1="-6.858" x2="-2.3622" y2="0.381" layer="21"/>
<rectangle x1="-1.778" y1="-6.858" x2="-0.9652" y2="-2.159" layer="21"/>
<rectangle x1="-0.4064" y1="-6.858" x2="0.4064" y2="0.381" layer="21"/>
<rectangle x1="0.9652" y1="-6.858" x2="1.778" y2="-2.159" layer="21"/>
<rectangle x1="2.3622" y1="-6.858" x2="3.175" y2="0.381" layer="21"/>
<rectangle x1="3.7338" y1="-6.858" x2="4.5466" y2="-2.159" layer="21"/>
<rectangle x1="5.1054" y1="-6.858" x2="5.9182" y2="0.381" layer="21"/>
<rectangle x1="6.5024" y1="-6.858" x2="7.3152" y2="-2.159" layer="21"/>
<rectangle x1="7.874" y1="-6.858" x2="8.6868" y2="0.381" layer="21"/>
<rectangle x1="9.2456" y1="-6.858" x2="10.0584" y2="-2.159" layer="21"/>
<rectangle x1="10.6426" y1="-6.858" x2="11.4554" y2="0.381" layer="21"/>
<rectangle x1="12.0142" y1="-6.858" x2="12.827" y2="-2.159" layer="21"/>
<rectangle x1="13.3858" y1="-6.858" x2="14.1986" y2="0.381" layer="21"/>
<rectangle x1="14.7828" y1="-6.858" x2="15.5956" y2="-2.159" layer="21"/>
<rectangle x1="16.1544" y1="-6.858" x2="16.9672" y2="0.381" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="M25">
<wire x1="1.27" y1="12.7" x2="2.54" y2="12.7" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="12.7" x2="-1.27" y2="12.7" width="0.6096" layer="94"/>
<wire x1="1.27" y1="7.62" x2="2.54" y2="7.62" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="10.16" x2="-1.27" y2="10.16" width="0.6096" layer="94"/>
<wire x1="1.27" y1="2.54" x2="2.54" y2="2.54" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="7.62" x2="-1.27" y2="7.62" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-2.54" x2="2.54" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="-1.27" y2="5.08" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-7.62" x2="2.54" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="-1.27" y2="2.54" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-12.7" x2="2.54" y2="-12.7" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-17.78" x2="2.54" y2="-17.78" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="-1.27" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="1.27" y1="10.16" x2="2.54" y2="10.16" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-5.08" x2="-1.27" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="1.27" y1="5.08" x2="2.54" y2="5.08" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-7.62" x2="-1.27" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-10.16" x2="-1.27" y2="-10.16" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-5.08" x2="2.54" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-12.7" x2="-1.27" y2="-12.7" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-10.16" x2="2.54" y2="-10.16" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-15.24" x2="-1.27" y2="-15.24" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-15.24" x2="2.54" y2="-15.24" width="0.6096" layer="94"/>
<wire x1="2.5226" y1="-20.8718" x2="4.0637" y2="-19.6312" width="0.4064" layer="94" curve="102.322193" cap="flat"/>
<wire x1="2.5226" y1="-20.8718" x2="-3.0654" y2="-19.6494" width="0.4064" layer="94"/>
<wire x1="-4.064" y1="-18.4088" x2="-3.0654" y2="-19.6494" width="0.4064" layer="94" curve="77.657889"/>
<wire x1="-4.064" y1="13.3288" x2="-4.064" y2="-18.4088" width="0.4064" layer="94"/>
<wire x1="-4.064" y1="13.3288" x2="-3.0654" y2="14.5694" width="0.4064" layer="94" curve="-77.657889"/>
<wire x1="4.064" y1="14.5512" x2="4.064" y2="-19.6312" width="0.4064" layer="94"/>
<wire x1="2.5226" y1="15.7918" x2="-3.0654" y2="14.5694" width="0.4064" layer="94"/>
<wire x1="2.5226" y1="15.7919" x2="4.064" y2="14.5512" width="0.4064" layer="94" curve="-102.337599" cap="flat"/>
<text x="-3.81" y="-23.495" size="1.778" layer="96">&gt;VALUE</text>
<text x="-3.81" y="16.51" size="1.778" layer="95">&gt;NAME</text>
<pin name="1" x="7.62" y="12.7" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="14" x="-7.62" y="12.7" visible="pad" length="middle" direction="pas"/>
<pin name="2" x="7.62" y="10.16" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="15" x="-7.62" y="10.16" visible="pad" length="middle" direction="pas"/>
<pin name="3" x="7.62" y="7.62" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="16" x="-7.62" y="7.62" visible="pad" length="middle" direction="pas"/>
<pin name="4" x="7.62" y="5.08" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="17" x="-7.62" y="5.08" visible="pad" length="middle" direction="pas"/>
<pin name="5" x="7.62" y="2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="18" x="-7.62" y="2.54" visible="pad" length="middle" direction="pas"/>
<pin name="6" x="7.62" y="0" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="19" x="-7.62" y="0" visible="pad" length="middle" direction="pas"/>
<pin name="7" x="7.62" y="-2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="20" x="-7.62" y="-2.54" visible="pad" length="middle" direction="pas"/>
<pin name="8" x="7.62" y="-5.08" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="21" x="-7.62" y="-5.08" visible="pad" length="middle" direction="pas"/>
<pin name="9" x="7.62" y="-7.62" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="22" x="-7.62" y="-7.62" visible="pad" length="middle" direction="pas"/>
<pin name="10" x="7.62" y="-10.16" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="23" x="-7.62" y="-10.16" visible="pad" length="middle" direction="pas"/>
<pin name="11" x="7.62" y="-12.7" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="24" x="-7.62" y="-12.7" visible="pad" length="middle" direction="pas"/>
<pin name="12" x="7.62" y="-15.24" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="25" x="-7.62" y="-15.24" visible="pad" length="middle" direction="pas"/>
<pin name="13" x="7.62" y="-17.78" visible="pad" length="middle" direction="pas" rot="R180"/>
</symbol>
<symbol name="GND">
<wire x1="-2.54" y1="0" x2="-0.635" y2="0" width="0.1524" layer="94"/>
<circle x="0" y="0" radius="0.635" width="0.1524" layer="94"/>
<text x="2.54" y="-0.762" size="1.778" layer="95">&gt;NAME</text>
<pin name="G1" x="-5.08" y="0" visible="off" length="short" direction="pas"/>
<pin name="G2" x="-2.54" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="M25HP" prefix="X" uservalue="yes">
<description>&lt;b&gt;SUB-D&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="M25" x="-5.08" y="0"/>
<gate name="M" symbol="GND" x="20.32" y="12.7" addlevel="request"/>
</gates>
<devices>
<device name="" package="M25HP">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="10" pad="10"/>
<connect gate="G$1" pin="11" pad="11"/>
<connect gate="G$1" pin="12" pad="12"/>
<connect gate="G$1" pin="13" pad="13"/>
<connect gate="G$1" pin="14" pad="14"/>
<connect gate="G$1" pin="15" pad="15"/>
<connect gate="G$1" pin="16" pad="16"/>
<connect gate="G$1" pin="17" pad="17"/>
<connect gate="G$1" pin="18" pad="18"/>
<connect gate="G$1" pin="19" pad="19"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="20" pad="20"/>
<connect gate="G$1" pin="21" pad="21"/>
<connect gate="G$1" pin="22" pad="22"/>
<connect gate="G$1" pin="23" pad="23"/>
<connect gate="G$1" pin="24" pad="24"/>
<connect gate="G$1" pin="25" pad="25"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
<connect gate="G$1" pin="8" pad="8"/>
<connect gate="G$1" pin="9" pad="9"/>
<connect gate="M" pin="G1" pad="G1"/>
<connect gate="M" pin="G2" pad="G2"/>
</connects>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="jet-lib">
<packages>
<package name="PDIP-32">
<wire x1="-1.524" y1="1.524" x2="16.764" y2="1.524" width="0.127" layer="21"/>
<wire x1="16.764" y1="1.524" x2="16.764" y2="-42.164" width="0.127" layer="21"/>
<wire x1="16.764" y1="-42.164" x2="-1.524" y2="-42.164" width="0.127" layer="21"/>
<wire x1="-1.524" y1="-42.164" x2="-1.524" y2="1.524" width="0.127" layer="21"/>
<pad name="D1/TX" x="0" y="-2.54" drill="0.8" diameter="1.778" shape="square"/>
<pad name="D0/RX" x="0" y="-5.08" drill="0.8" diameter="1.778"/>
<pad name="RESET" x="0" y="-7.62" drill="0.8" diameter="1.778"/>
<pad name="GND" x="0" y="-10.16" drill="0.8" diameter="1.778"/>
<pad name="D2" x="0" y="-12.7" drill="0.8" diameter="1.778"/>
<pad name="D3" x="0" y="-15.24" drill="0.8" diameter="1.778"/>
<pad name="D4" x="0" y="-17.78" drill="0.8" diameter="1.778"/>
<pad name="D5" x="0" y="-20.32" drill="0.8" diameter="1.778"/>
<pad name="D6" x="0" y="-22.86" drill="0.8" diameter="1.778"/>
<pad name="D7" x="0" y="-25.4" drill="0.8" diameter="1.778"/>
<pad name="D8" x="0" y="-27.94" drill="0.8" diameter="1.778"/>
<pad name="D9" x="0" y="-30.48" drill="0.8" diameter="1.778"/>
<pad name="D10" x="0" y="-33.02" drill="0.8" diameter="1.778"/>
<pad name="D11" x="0" y="-35.56" drill="0.8" diameter="1.778"/>
<pad name="D12" x="0" y="-38.1" drill="0.8" diameter="1.778"/>
<pad name="VIN" x="15.24" y="-2.54" drill="0.8" diameter="1.778"/>
<pad name="GND1" x="15.24" y="-5.08" drill="0.8" diameter="1.778"/>
<pad name="RESET1" x="15.24" y="-7.62" drill="0.8" diameter="1.778"/>
<pad name="+5V" x="15.24" y="-10.16" drill="0.8" diameter="1.778"/>
<pad name="A0" x="15.24" y="-12.7" drill="0.8" diameter="1.778"/>
<pad name="A1" x="15.24" y="-15.24" drill="0.8" diameter="1.778"/>
<pad name="A2" x="15.24" y="-17.78" drill="0.8" diameter="1.778"/>
<pad name="A3" x="15.24" y="-20.32" drill="0.8" diameter="1.778"/>
<pad name="A4" x="15.24" y="-22.86" drill="0.8" diameter="1.778"/>
<pad name="A5" x="15.24" y="-25.4" drill="0.8" diameter="1.778"/>
<pad name="A6" x="15.24" y="-27.94" drill="0.8" diameter="1.778"/>
<pad name="A7" x="15.24" y="-30.48" drill="0.8" diameter="1.778"/>
<pad name="AREF" x="15.24" y="-33.02" drill="0.8" diameter="1.778"/>
<pad name="3V3" x="15.24" y="-35.56" drill="0.8" diameter="1.778"/>
<pad name="D13" x="15.24" y="-38.1" drill="0.8" diameter="1.778"/>
<text x="-0.762" y="2.286" size="1.27" layer="25">Name</text>
<text x="-1.27" y="-44.704" size="1.27" layer="25">Value</text>
<hole x="0" y="0" drill="1.778"/>
<hole x="0" y="-40.64" drill="1.778"/>
<hole x="15.24" y="-40.64" drill="1.778"/>
<hole x="15.24" y="0" drill="1.778"/>
</package>
</packages>
<symbols>
<symbol name="ARDUINONANO">
<wire x1="0" y1="-76.2" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="20.32" y2="0" width="0.254" layer="94"/>
<wire x1="20.32" y1="0" x2="20.32" y2="-76.2" width="0.254" layer="94"/>
<wire x1="20.32" y1="-76.2" x2="0" y2="-76.2" width="0.254" layer="94"/>
<pin name="D1/TX" x="-5.08" y="-2.54" visible="pin" length="middle"/>
<pin name="D0/RX" x="-5.08" y="-7.62" visible="pin" length="middle"/>
<pin name="RESET" x="-5.08" y="-12.7" visible="pin" length="middle"/>
<pin name="GND" x="-5.08" y="-17.78" visible="pin" length="middle"/>
<pin name="D2" x="-5.08" y="-22.86" visible="pin" length="middle"/>
<pin name="D3" x="-5.08" y="-27.94" visible="pin" length="middle"/>
<pin name="D4" x="-5.08" y="-33.02" visible="pin" length="middle"/>
<pin name="D5" x="-5.08" y="-38.1" visible="pin" length="middle"/>
<pin name="D6" x="-5.08" y="-43.18" visible="pin" length="middle"/>
<pin name="D7" x="-5.08" y="-48.26" visible="pin" length="middle"/>
<pin name="D8" x="-5.08" y="-53.34" visible="pin" length="middle"/>
<pin name="D9" x="-5.08" y="-58.42" visible="pin" length="middle"/>
<pin name="D10" x="-5.08" y="-63.5" visible="pin" length="middle"/>
<pin name="D11" x="-5.08" y="-68.58" visible="pin" length="middle"/>
<pin name="D12" x="-5.08" y="-73.66" visible="pin" length="middle"/>
<pin name="VIN" x="25.4" y="-2.54" visible="pin" length="middle" rot="R180"/>
<pin name="GND1" x="25.4" y="-7.62" visible="pin" length="middle" rot="R180"/>
<pin name="RESET1" x="25.4" y="-12.7" visible="pin" length="middle" rot="R180"/>
<pin name="+5V" x="25.4" y="-17.78" visible="pin" length="middle" rot="R180"/>
<pin name="A7" x="25.4" y="-22.86" visible="pin" length="middle" rot="R180"/>
<pin name="A6" x="25.4" y="-27.94" visible="pin" length="middle" rot="R180"/>
<pin name="A5" x="25.4" y="-33.02" visible="pin" length="middle" rot="R180"/>
<pin name="A4" x="25.4" y="-38.1" visible="pin" length="middle" rot="R180"/>
<pin name="A3" x="25.4" y="-43.18" visible="pin" length="middle" rot="R180"/>
<pin name="A2" x="25.4" y="-48.26" visible="pin" length="middle" rot="R180"/>
<pin name="A1" x="25.4" y="-53.34" visible="pin" length="middle" rot="R180"/>
<pin name="A0" x="25.4" y="-58.42" visible="pin" length="middle" rot="R180"/>
<pin name="AREF" x="25.4" y="-63.5" visible="pin" length="middle" rot="R180"/>
<pin name="3V3" x="25.4" y="-68.58" visible="pin" length="middle" rot="R180"/>
<pin name="D13" x="25.4" y="-73.66" visible="pin" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="ARDUINONANO">
<gates>
<gate name="G$1" symbol="ARDUINONANO" x="-10.16" y="38.1"/>
</gates>
<devices>
<device name="" package="PDIP-32">
<connects>
<connect gate="G$1" pin="+5V" pad="+5V"/>
<connect gate="G$1" pin="3V3" pad="3V3"/>
<connect gate="G$1" pin="A0" pad="A7"/>
<connect gate="G$1" pin="A1" pad="A6"/>
<connect gate="G$1" pin="A2" pad="A5"/>
<connect gate="G$1" pin="A3" pad="A4"/>
<connect gate="G$1" pin="A4" pad="A3"/>
<connect gate="G$1" pin="A5" pad="A2"/>
<connect gate="G$1" pin="A6" pad="A1"/>
<connect gate="G$1" pin="A7" pad="A0"/>
<connect gate="G$1" pin="AREF" pad="AREF"/>
<connect gate="G$1" pin="D0/RX" pad="D0/RX"/>
<connect gate="G$1" pin="D1/TX" pad="D1/TX"/>
<connect gate="G$1" pin="D10" pad="D10"/>
<connect gate="G$1" pin="D11" pad="D11"/>
<connect gate="G$1" pin="D12" pad="D12"/>
<connect gate="G$1" pin="D13" pad="D13"/>
<connect gate="G$1" pin="D2" pad="D2"/>
<connect gate="G$1" pin="D3" pad="D3"/>
<connect gate="G$1" pin="D4" pad="D4"/>
<connect gate="G$1" pin="D5" pad="D5"/>
<connect gate="G$1" pin="D6" pad="D6"/>
<connect gate="G$1" pin="D7" pad="D7"/>
<connect gate="G$1" pin="D8" pad="D8"/>
<connect gate="G$1" pin="D9" pad="D9"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="GND1" pad="GND1"/>
<connect gate="G$1" pin="RESET" pad="RESET"/>
<connect gate="G$1" pin="RESET1" pad="RESET1"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
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
<part name="GND1" library="supply1" deviceset="GND" device=""/>
<part name="FRAME1" library="frames" deviceset="DINA4_L" device=""/>
<part name="GND2" library="supply1" deviceset="GND" device=""/>
<part name="GND3" library="supply1" deviceset="GND" device=""/>
<part name="X1" library="con-subd" deviceset="M25HP" device=""/>
<part name="U$1" library="jet-lib" deviceset="ARDUINONANO" device=""/>
</parts>
<sheets>
<sheet>
<plain>
<text x="86.36" y="134.62" size="1.778" layer="91">Arduino Nano V3</text>
<text x="172.72" y="127" size="1.778" layer="91">Amiga Parallel Port</text>
<text x="91.44" y="165.1" size="5.08" layer="94" font="vector">plipbox nano v1.1</text>
<text x="104.14" y="157.48" size="3.81" layer="94" font="vector">by lallafa 2013</text>
</plain>
<instances>
<instance part="GND1" gate="1" x="200.66" y="76.2"/>
<instance part="FRAME1" gate="G$1" x="0" y="0"/>
<instance part="FRAME1" gate="G$2" x="162.56" y="0"/>
<instance part="GND2" gate="1" x="58.42" y="116.84"/>
<instance part="GND3" gate="1" x="132.08" y="119.38"/>
<instance part="X1" gate="G$1" x="182.88" y="101.6" rot="MR0"/>
<instance part="U$1" gate="G$1" x="86.36" y="132.08"/>
</instances>
<busses>
</busses>
<nets>
<net name="D0" class="0">
<segment>
<wire x1="175.26" y1="111.76" x2="154.94" y2="111.76" width="0.1524" layer="91"/>
<label x="154.94" y="111.76" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="2"/>
</segment>
<segment>
<pinref part="U$1" gate="G$1" pin="A0"/>
<wire x1="111.76" y1="73.66" x2="137.16" y2="73.66" width="0.1524" layer="91"/>
<label x="114.3" y="73.66" size="1.778" layer="95"/>
</segment>
</net>
<net name="D1" class="0">
<segment>
<wire x1="175.26" y1="109.22" x2="154.94" y2="109.22" width="0.1524" layer="91"/>
<label x="154.94" y="109.22" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="3"/>
</segment>
<segment>
<pinref part="U$1" gate="G$1" pin="A1"/>
<wire x1="111.76" y1="78.74" x2="137.16" y2="78.74" width="0.1524" layer="91"/>
<label x="114.3" y="78.74" size="1.778" layer="95"/>
</segment>
</net>
<net name="D2" class="0">
<segment>
<wire x1="175.26" y1="106.68" x2="154.94" y2="106.68" width="0.1524" layer="91"/>
<label x="154.94" y="106.68" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="4"/>
</segment>
<segment>
<wire x1="111.76" y1="83.82" x2="137.16" y2="83.82" width="0.1524" layer="91"/>
<label x="114.3" y="83.82" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="A2"/>
</segment>
</net>
<net name="D3" class="0">
<segment>
<wire x1="175.26" y1="104.14" x2="154.94" y2="104.14" width="0.1524" layer="91"/>
<label x="154.94" y="104.14" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="5"/>
</segment>
<segment>
<wire x1="111.76" y1="88.9" x2="137.16" y2="88.9" width="0.1524" layer="91"/>
<label x="114.3" y="88.9" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="A3"/>
</segment>
</net>
<net name="D4" class="0">
<segment>
<wire x1="175.26" y1="101.6" x2="154.94" y2="101.6" width="0.1524" layer="91"/>
<label x="154.94" y="101.6" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="6"/>
</segment>
<segment>
<wire x1="111.76" y1="93.98" x2="137.16" y2="93.98" width="0.1524" layer="91"/>
<label x="114.3" y="93.98" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="A4"/>
</segment>
</net>
<net name="D5" class="0">
<segment>
<wire x1="175.26" y1="99.06" x2="154.94" y2="99.06" width="0.1524" layer="91"/>
<label x="154.94" y="99.06" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="7"/>
</segment>
<segment>
<wire x1="111.76" y1="99.06" x2="137.16" y2="99.06" width="0.1524" layer="91"/>
<label x="114.3" y="99.06" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="A5"/>
</segment>
</net>
<net name="D6" class="0">
<segment>
<wire x1="175.26" y1="96.52" x2="154.94" y2="96.52" width="0.1524" layer="91"/>
<label x="154.94" y="96.52" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="8"/>
</segment>
<segment>
<wire x1="81.28" y1="88.9" x2="55.88" y2="88.9" width="0.1524" layer="91"/>
<label x="55.88" y="88.9" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D6"/>
</segment>
</net>
<net name="D7" class="0">
<segment>
<wire x1="175.26" y1="93.98" x2="154.94" y2="93.98" width="0.1524" layer="91"/>
<label x="154.94" y="93.98" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="9"/>
</segment>
<segment>
<wire x1="81.28" y1="83.82" x2="55.88" y2="83.82" width="0.1524" layer="91"/>
<label x="55.88" y="83.82" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D7"/>
</segment>
</net>
<net name="/ACK" class="0">
<segment>
<wire x1="175.26" y1="91.44" x2="154.94" y2="91.44" width="0.1524" layer="91"/>
<label x="154.94" y="91.44" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="10"/>
</segment>
<segment>
<wire x1="81.28" y1="78.74" x2="55.88" y2="78.74" width="0.1524" layer="91"/>
<label x="55.88" y="78.74" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D8"/>
</segment>
</net>
<net name="BUSY" class="0">
<segment>
<wire x1="175.26" y1="88.9" x2="154.94" y2="88.9" width="0.1524" layer="91"/>
<label x="154.94" y="88.9" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="11"/>
</segment>
<segment>
<wire x1="81.28" y1="99.06" x2="55.88" y2="99.06" width="0.1524" layer="91"/>
<label x="55.88" y="99.06" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D4"/>
</segment>
</net>
<net name="POUT" class="0">
<segment>
<wire x1="175.26" y1="86.36" x2="154.94" y2="86.36" width="0.1524" layer="91"/>
<label x="154.94" y="86.36" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="12"/>
</segment>
<segment>
<wire x1="81.28" y1="93.98" x2="55.88" y2="93.98" width="0.1524" layer="91"/>
<label x="55.88" y="93.98" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D5"/>
</segment>
</net>
<net name="SELECT" class="0">
<segment>
<wire x1="175.26" y1="83.82" x2="154.94" y2="83.82" width="0.1524" layer="91"/>
<label x="154.94" y="83.82" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="13"/>
</segment>
<segment>
<wire x1="81.28" y1="73.66" x2="55.88" y2="73.66" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="D9"/>
<label x="55.88" y="73.66" size="1.778" layer="95"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<wire x1="190.5" y1="106.68" x2="200.66" y2="106.68" width="0.1524" layer="91"/>
<wire x1="200.66" y1="106.68" x2="200.66" y2="104.14" width="0.1524" layer="91"/>
<wire x1="200.66" y1="104.14" x2="200.66" y2="101.6" width="0.1524" layer="91"/>
<wire x1="200.66" y1="101.6" x2="200.66" y2="99.06" width="0.1524" layer="91"/>
<wire x1="200.66" y1="99.06" x2="200.66" y2="96.52" width="0.1524" layer="91"/>
<wire x1="200.66" y1="96.52" x2="200.66" y2="93.98" width="0.1524" layer="91"/>
<wire x1="200.66" y1="93.98" x2="200.66" y2="78.74" width="0.1524" layer="91"/>
<wire x1="190.5" y1="104.14" x2="200.66" y2="104.14" width="0.1524" layer="91"/>
<wire x1="190.5" y1="101.6" x2="200.66" y2="101.6" width="0.1524" layer="91"/>
<wire x1="190.5" y1="99.06" x2="200.66" y2="99.06" width="0.1524" layer="91"/>
<wire x1="190.5" y1="96.52" x2="200.66" y2="96.52" width="0.1524" layer="91"/>
<wire x1="190.5" y1="93.98" x2="200.66" y2="93.98" width="0.1524" layer="91"/>
<junction x="200.66" y="104.14"/>
<junction x="200.66" y="101.6"/>
<junction x="200.66" y="99.06"/>
<junction x="200.66" y="96.52"/>
<junction x="200.66" y="93.98"/>
<pinref part="GND1" gate="1" pin="GND"/>
<pinref part="X1" gate="G$1" pin="17"/>
<pinref part="X1" gate="G$1" pin="18"/>
<pinref part="X1" gate="G$1" pin="19"/>
<pinref part="X1" gate="G$1" pin="20"/>
<pinref part="X1" gate="G$1" pin="21"/>
<pinref part="X1" gate="G$1" pin="22"/>
</segment>
<segment>
<wire x1="81.28" y1="114.3" x2="68.58" y2="114.3" width="0.1524" layer="91"/>
<wire x1="68.58" y1="114.3" x2="68.58" y2="119.38" width="0.1524" layer="91"/>
<wire x1="68.58" y1="119.38" x2="58.42" y2="119.38" width="0.1524" layer="91"/>
<pinref part="GND2" gate="1" pin="GND"/>
<pinref part="U$1" gate="G$1" pin="GND"/>
</segment>
<segment>
<wire x1="111.76" y1="124.46" x2="129.54" y2="124.46" width="0.1524" layer="91"/>
<wire x1="129.54" y1="124.46" x2="132.08" y2="124.46" width="0.1524" layer="91"/>
<wire x1="132.08" y1="124.46" x2="132.08" y2="121.92" width="0.1524" layer="91"/>
<pinref part="GND3" gate="1" pin="GND"/>
<pinref part="U$1" gate="G$1" pin="GND1"/>
</segment>
</net>
<net name="/STROBE" class="0">
<segment>
<wire x1="175.26" y1="114.3" x2="154.94" y2="114.3" width="0.1524" layer="91"/>
<label x="154.94" y="114.3" size="1.778" layer="95"/>
<pinref part="X1" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="81.28" y1="104.14" x2="55.88" y2="104.14" width="0.1524" layer="91"/>
<label x="55.88" y="104.14" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="D3"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
