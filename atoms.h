#ifndef APPS_ATOM_DEFS_H
#define APPS_ATOM_DEFS_H

#include "apps/i18n.h"
#include "stddef.h"

enum AtomType : uint8_t {
  UNKNOWN = 0,
  ALKALI_METAL = 1,
  ALKALI_EARTH_METAL = 2,
  LANTHANIDE = 3,
  ACTINIDE = 4,
  TRANSITION_METAL = 5,
  POST_TRANSITION_METAL = 6,
  METALLOID = 7,
  HALOGEN = 8,
  REACTIVE_NONMETAL = 9,
  NOBLE_GAS = 10
};

const I18n::Message AtomicI18nForType[] = {
  I18n::Message::AtomTypeUNKNOWN,
  I18n::Message::AtomTypeALKALIMETAL,
  I18n::Message::AtomTypeALKALIEARTHMETAL,
  I18n::Message::AtomTypeLANTHANIDE,
  I18n::Message::AtomTypeACTINIDE,
  I18n::Message::AtomTypeTRANSITIONMETAL,
  I18n::Message::AtomTypePOSTTRANSITIONMETAL,
  I18n::Message::AtomTypeMETALLOID,
  I18n::Message::AtomTypeHALLOGEN,
  I18n::Message::AtomTypeREACTIVENONMETAL,
  I18n::Message::AtomTypeNOBLEGAS
};

struct AtomDef {
  uint8_t num;
  uint8_t x;
  uint8_t y;
  AtomType type;
  I18n::Message name;
  const char* symbol;
  uint8_t neutrons;
  double mass;
  double electroneg;
  double ionisation;
  double atomicRadius;
  double electronAffinity;
  double meltingPoint;
  double boilingPoint;
  double density;
};

const AtomDef atomsdefs[] = {
  {1,0,0,REACTIVE_NONMETAL,I18n::Message::AtomName_Hydrogen,"H",0,1.008,2.2,13.598,120,0.754,13.81,20.28,8.988e-05},
  {2,17,0,NOBLE_GAS,I18n::Message::AtomName_Helium,"He",2,4.0026,-1,24.587,140,-1,0.95,4.22,0.0001785},
  {3,0,1,ALKALI_METAL,I18n::Message::AtomName_Lithium,"Li",4,7,0.98,5.392,182,0.618,453.65,1615,0.534},
  {4,1,1,ALKALI_EARTH_METAL,I18n::Message::AtomName_Beryllium,"Be",5,9.01218,1.57,9.323,153,-1,1560,2744,1.85},
  {5,12,1,METALLOID,I18n::Message::AtomName_Boron,"B",6,10.81,2.04,8.298,192,0.277,2348,4273,2.37},
  {6,13,1,REACTIVE_NONMETAL,I18n::Message::AtomName_Carbon,"C",6,12.011,2.55,11.26,170,1.263,3823,4098,2.267},
  {7,14,1,REACTIVE_NONMETAL,I18n::Message::AtomName_Nitrogen,"N",7,14.007,3.04,14.534,155,-1,63.15,77.36,0.0012506},
  {8,15,1,REACTIVE_NONMETAL,I18n::Message::AtomName_Oxygen,"O",8,15.999,3.44,13.618,152,1.461,54.36,90.2,0.001429},
  {9,16,1,HALOGEN,I18n::Message::AtomName_Fluorine,"F",10,18.9984,3.98,17.423,135,3.339,53.53,85.03,0.001696},
  {10,17,1,NOBLE_GAS,I18n::Message::AtomName_Neon,"Ne",10,20.18,-1,21.565,154,-1,24.56,27.07,0.0008999},
  {11,0,2,ALKALI_METAL,I18n::Message::AtomName_Sodium,"Na",12,22.9898,0.93,5.139,227,0.548,370.95,1156,0.97},
  {12,1,2,ALKALI_EARTH_METAL,I18n::Message::AtomName_Magnesium,"Mg",12,24.305,1.31,7.646,173,-1,923,1363,1.74},
  {13,12,2,POST_TRANSITION_METAL,I18n::Message::AtomName_Aluminium,"Al",14,26.9815,1.61,5.986,184,0.441,933.437,2792,2.7},
  {14,13,2,METALLOID,I18n::Message::AtomName_Silicon,"Si",14,28.085,1.9,8.152,210,1.385,1687,3538,2.3296},
  {15,14,2,REACTIVE_NONMETAL,I18n::Message::AtomName_Phosphorus,"P",16,30.9738,2.19,10.487,180,0.746,317.3,553.65,1.82},
  {16,15,2,REACTIVE_NONMETAL,I18n::Message::AtomName_Sulfur,"S",16,32.07,2.58,10.36,180,2.077,388.36,717.75,2.067},
  {17,16,2,HALOGEN,I18n::Message::AtomName_Chlorine,"Cl",18,35.45,3.16,12.968,175,3.617,171.65,239.11,0.003214},
  {18,17,2,NOBLE_GAS,I18n::Message::AtomName_Argon,"Ar",22,39.9,-1,15.76,188,-1,83.8,87.3,0.0017837},
  {19,0,3,ALKALI_METAL,I18n::Message::AtomName_Potassium,"K",20,39.0983,0.82,4.341,275,0.501,336.53,1032,0.89},
  {20,1,3,ALKALI_EARTH_METAL,I18n::Message::AtomName_Calcium,"Ca",20,40.08,1,6.113,231,-1,1115,1757,1.54},
  {21,2,3,TRANSITION_METAL,I18n::Message::AtomName_Scandium,"Sc",24,44.9559,1.36,6.561,211,0.188,1814,3109,2.99},
  {22,3,3,TRANSITION_METAL,I18n::Message::AtomName_Titanium,"Ti",26,47.867,1.54,6.828,187,0.079,1941,3560,4.5},
  {23,4,3,TRANSITION_METAL,I18n::Message::AtomName_Vanadium,"V",28,50.9415,1.63,6.746,179,0.525,2183,3680,6},
  {24,5,3,TRANSITION_METAL,I18n::Message::AtomName_Chromium,"Cr",28,51.996,1.66,6.767,189,0.666,2180,2944,7.15},
  {25,6,3,TRANSITION_METAL,I18n::Message::AtomName_Manganese,"Mn",30,54.938,1.55,7.434,197,-1,1519,2334,7.3},
  {26,7,3,TRANSITION_METAL,I18n::Message::AtomName_Iron,"Fe",30,55.84,1.83,7.902,194,0.163,1811,3134,7.874},
  {27,8,3,TRANSITION_METAL,I18n::Message::AtomName_Cobalt,"Co",32,58.9332,1.88,7.881,192,0.661,1768,3200,8.86},
  {28,9,3,TRANSITION_METAL,I18n::Message::AtomName_Nickel,"Ni",31,58.693,1.91,7.64,163,1.156,1728,3186,8.912},
  {29,10,3,TRANSITION_METAL,I18n::Message::AtomName_Copper,"Cu",34,63.55,1.9,7.726,140,1.228,1357.77,2835,8.933},
  {30,11,3,POST_TRANSITION_METAL,I18n::Message::AtomName_Zinc,"Zn",34,65.4,1.65,9.394,139,-1,692.68,1180,7.134},
  {31,12,3,POST_TRANSITION_METAL,I18n::Message::AtomName_Gallium,"Ga",38,69.723,1.81,5.999,187,0.3,302.91,2477,5.91},
  {32,13,3,METALLOID,I18n::Message::AtomName_Germanium,"Ge",42,72.63,2.01,7.9,211,1.35,1211.4,3106,5.323},
  {33,14,3,METALLOID,I18n::Message::AtomName_Arsenic,"As",42,74.9216,2.18,9.815,185,0.81,1090,887,5.776},
  {34,15,3,REACTIVE_NONMETAL,I18n::Message::AtomName_Selenium,"Se",46,78.97,2.55,9.752,190,2.021,493.65,958,4.809},
  {35,16,3,HALOGEN,I18n::Message::AtomName_Bromine,"Br",44,79.9,2.96,11.814,183,3.365,265.95,331.95,3.11},
  {36,17,3,NOBLE_GAS,I18n::Message::AtomName_Krypton,"Kr",48,83.8,3,14,202,-1,115.79,119.93,0.003733},
  {37,0,4,ALKALI_METAL,I18n::Message::AtomName_Rubidium,"Rb",48,85.468,0.82,4.177,303,0.468,312.46,961,1.53},
  {38,1,4,ALKALI_EARTH_METAL,I18n::Message::AtomName_Strontium,"Sr",50,87.62,0.95,5.695,249,-1,1050,1655,2.64},
  {39,2,4,TRANSITION_METAL,I18n::Message::AtomName_Yttrium,"Y",50,88.9058,1.22,6.217,219,0.307,1795,3618,4.47},
  {40,3,4,TRANSITION_METAL,I18n::Message::AtomName_Zirconium,"Zr",50,91.22,1.33,6.634,186,0.426,2128,4682,6.52},
  {41,4,4,TRANSITION_METAL,I18n::Message::AtomName_Niobium,"Nb",52,92.9064,1.6,6.759,207,0.893,2750,5017,8.57},
  {42,5,4,TRANSITION_METAL,I18n::Message::AtomName_Molybdenum,"Mo",56,95.95,2.16,7.092,209,0.746,2896,4912,10.2},
  {43,6,4,TRANSITION_METAL,I18n::Message::AtomName_Technetium,"Tc",56,96.9064,1.9,7.28,209,0.55,2430,4538,11},
  {44,7,4,TRANSITION_METAL,I18n::Message::AtomName_Ruthemium,"Ru",58,101.1,2.2,7.361,207,1.05,2607,4423,12.1},
  {45,8,4,TRANSITION_METAL,I18n::Message::AtomName_Rhodium,"Rh",58,102.906,2.28,7.459,195,1.137,2237,3968,12.4},
  {46,9,4,TRANSITION_METAL,I18n::Message::AtomName_Palladium,"Pd",60,106.42,2.2,8.337,202,0.557,1828.05,3236,12},
  {47,10,4,TRANSITION_METAL,I18n::Message::AtomName_Silver,"Ag",60,107.868,1.93,7.576,172,1.302,1234.93,2435,10.501},
  {48,11,4,POST_TRANSITION_METAL,I18n::Message::AtomName_Cadmium,"Cd",66,112.41,1.69,8.994,158,-1,594.22,1040,8.69},
  {49,12,4,POST_TRANSITION_METAL,I18n::Message::AtomName_Indium,"In",66,114.818,1.78,5.786,193,0.3,429.75,2345,7.31},
  {50,13,4,POST_TRANSITION_METAL,I18n::Message::AtomName_Tin,"Sn",70,118.71,1.96,7.344,217,1.2,505.08,2875,7.287},
  {51,14,4,METALLOID,I18n::Message::AtomName_Antimony,"Sb",70,121.76,2.05,8.64,206,1.07,903.78,1860,6.685},
  {52,15,4,METALLOID,I18n::Message::AtomName_Tellurium,"Te",78,127.6,2.1,9.01,206,1.971,722.66,1261,6.232},
  {53,16,4,HALOGEN,I18n::Message::AtomName_Indine,"I",74,126.904,2.66,10.451,198,3.059,386.85,457.55,4.93},
  {54,17,4,NOBLE_GAS,I18n::Message::AtomName_Xenon,"Xe",78,131.29,2.6,12.13,216,-1,161.36,165.03,0.005887},
  {55,0,5,ALKALI_METAL,I18n::Message::AtomName_Caesium,"Cs",78,132.905,0.79,3.894,343,0.472,301.59,944,1.93},
  {56,1,5,ALKALI_EARTH_METAL,I18n::Message::AtomName_Barium,"Ba",81,137.33,0.89,5.212,268,-1,1000,2170,3.62},
  {57,3,8,LANTHANIDE,I18n::Message::AtomName_Lanthanum,"La",82,138.905,1.1,5.577,240,0.5,1191,3737,6.15},
  {58,4,8,LANTHANIDE,I18n::Message::AtomName_Cerium,"Ce",82,140.116,1.12,5.539,235,0.5,1071,3697,6.77},
  {59,5,8,LANTHANIDE,I18n::Message::AtomName_Praseodymium,"Pr",82,140.908,1.13,5.464,239,-1,1204,3793,6.77},
  {60,6,8,LANTHANIDE,I18n::Message::AtomName_Neodymium,"Nd",84,144.24,1.14,5.525,229,-1,1294,3347,7.01},
  {61,7,8,LANTHANIDE,I18n::Message::AtomName_Promethium,"Pm",84,144.913,-1,5.55,236,-1,1315,3273,7.26},
  {62,8,8,LANTHANIDE,I18n::Message::AtomName_Samarium,"Sm",88,150.4,1.17,5.644,229,-1,1347,2067,7.52},
  {63,9,8,LANTHANIDE,I18n::Message::AtomName_Europium,"Eu",89,151.964,-1,5.67,233,-1,1095,1802,5.24},
  {64,10,8,LANTHANIDE,I18n::Message::AtomName_Gadolinium,"Gd",93,157.25,1.2,6.15,237,-1,1586,3546,7.9},
  {65,11,8,LANTHANIDE,I18n::Message::AtomName_Terbium,"Tb",94,158.925,-1,5.864,221,-1,1629,3503,8.23},
  {66,12,8,LANTHANIDE,I18n::Message::AtomName_Dyxprosium,"Dy",97,162.5,1.22,5.939,229,-1,1685,2840,8.55},
  {67,13,8,LANTHANIDE,I18n::Message::AtomName_Holmium,"Ho",98,164.93,1.23,6.022,216,-1,1747,2973,8.8},
  {68,14,8,LANTHANIDE,I18n::Message::AtomName_Erbium,"Er",99,167.26,1.24,6.108,235,-1,1802,3141,9.07},
  {69,15,8,LANTHANIDE,I18n::Message::AtomName_Thulium,"Tm",100,168.934,1.25,6.184,227,-1,1818,2223,9.32},
  {70,16,8,LANTHANIDE,I18n::Message::AtomName_Ytterbium,"Yb",103,173.05,-1,6.254,242,-1,1092,1469,6.9},
  {71,17,8,LANTHANIDE,I18n::Message::AtomName_Lutetium,"Lu",104,174.967,1.27,5.426,221,-1,1936,3675,9.84},
  {72,3,5,TRANSITION_METAL,I18n::Message::AtomName_Hafnium,"Hf",106,178.49,1.3,6.825,212,-1,2506,4876,13.3},
  {73,4,5,TRANSITION_METAL,I18n::Message::AtomName_Tantalum,"Ta",108,180.948,1.5,7.89,217,0.322,3290,5731,16.4},
  {74,5,5,TRANSITION_METAL,I18n::Message::AtomName_Tungsten,"W",110,183.84,2.36,7.98,210,0.815,3695,5828,19.3},
  {75,6,5,TRANSITION_METAL,I18n::Message::AtomName_Rhenium,"Re",111,186.207,1.9,7.88,217,0.15,3459,5869,20.8},
  {76,7,5,TRANSITION_METAL,I18n::Message::AtomName_Osmium,"Os",114,190.2,2.2,8.7,216,1.1,3306,5285,22.57},
  {77,8,5,TRANSITION_METAL,I18n::Message::AtomName_Iridium,"Ir",115,192.22,2.2,9.1,202,1.565,2719,4701,22.42},
  {78,9,5,TRANSITION_METAL,I18n::Message::AtomName_Platinum,"Pt",117,195.08,2.28,9,209,2.128,2041.55,4098,21.46},
  {79,10,5,TRANSITION_METAL,I18n::Message::AtomName_Gold,"Au",118,196.967,2.54,9.226,166,2.309,1337.33,3129,19.282},
  {80,11,5,POST_TRANSITION_METAL,I18n::Message::AtomName_Mercury,"Hg",121,200.59,2,10.438,209,-1,234.32,629.88,13.5336},
  {81,12,5,POST_TRANSITION_METAL,I18n::Message::AtomName_Thalium,"Tl",123,204.383,1.62,6.108,196,0.2,577,1746,11.8},
  {82,13,5,POST_TRANSITION_METAL,I18n::Message::AtomName_Lead,"Pb",125,207,2.33,7.417,202,0.36,600.61,2022,11.342},
  {83,14,5,POST_TRANSITION_METAL,I18n::Message::AtomName_Bismuth,"Bi",126,208.98,2.02,7.289,207,0.946,544.55,1837,9.807},
  {84,15,5,POST_TRANSITION_METAL,I18n::Message::AtomName_Polonium,"Po",125,208.982,2,8.417,197,1.9,527,1235,9.32},
  {85,16,5,HALOGEN,I18n::Message::AtomName_Astatine,"At",125,209.987,2.2,9.5,202,2.8,575,-1,7},
  {86,17,5,NOBLE_GAS,I18n::Message::AtomName_Radon,"Rn",136,222.018,-1,10.745,220,-1,202,211.45,0.00973},
  {87,0,6,ALKALI_METAL,I18n::Message::AtomName_Francium,"Fr",136,223.02,0.7,3.9,348,0.47,300,-1,-1},
  {88,1,6,ALKALI_EARTH_METAL,I18n::Message::AtomName_Radium,"Ra",138,226.025,0.9,5.279,283,-1,973,1413,5},
  {89,3,9,ACTINIDE,I18n::Message::AtomName_Actinium,"Ac",138,227.028,1.1,5.17,260,-1,1324,3471,10.07},
  {90,4,9,ACTINIDE,I18n::Message::AtomName_Thorium,"Th",142,232.038,1.3,6.08,237,-1,2023,5061,11.72},
  {91,5,9,ACTINIDE,I18n::Message::AtomName_Protactinium,"Pa",140,231.036,1.5,5.89,243,-1,1845,-1,15.37},
  {92,6,9,ACTINIDE,I18n::Message::AtomName_Uranium,"U",146,238.029,1.38,6.194,240,-1,1408,4404,18.95},
  {93,7,9,ACTINIDE,I18n::Message::AtomName_Neptunium,"Np",144,237.048,1.36,6.266,221,-1,917,4175,20.25},
  {94,8,9,ACTINIDE,I18n::Message::AtomName_Plutonium,"Pu",150,244.064,1.28,6.06,243,-1,913,3501,19.84},
  {95,9,9,ACTINIDE,I18n::Message::AtomName_Americium,"Am",146,243.061,1.3,5.993,244,-1,1449,2284,13.69},
  {96,10,9,ACTINIDE,I18n::Message::AtomName_Curium,"Cm",151,247.07,1.3,6.02,245,-1,1618,3400,13.51},
  {97,11,9,ACTINIDE,I18n::Message::AtomName_Berkelium,"Bk",150,247.07,1.3,6.23,244,-1,1323,-1,14},
  {98,12,9,ACTINIDE,I18n::Message::AtomName_Californium,"Cf",153,251.08,1.3,6.3,245,-1,1173,-1,-1},
  {99,13,9,ACTINIDE,I18n::Message::AtomName_Einsteinium,"Es",153,252.083,1.3,6.42,245,-1,1133,-1,-1},
  {100,14,9,ACTINIDE,I18n::Message::AtomName_Fermium,"Fm",157,257.095,1.3,6.5,-1,-1,1800,-1,-1},
  {101,15,9,ACTINIDE,I18n::Message::AtomName_Mendelevium,"Md",157,258.098,1.3,6.58,-1,-1,1100,-1,-1},
  {102,16,9,ACTINIDE,I18n::Message::AtomName_Nobelium,"No",157,259.101,1.3,6.65,-1,-1,1100,-1,-1},
  {103,17,9,ACTINIDE,I18n::Message::AtomName_Lawrencium,"Lr",163,266.12,1.3,-1,-1,-1,1900,-1,-1},
  {104,3,6,TRANSITION_METAL,I18n::Message::AtomName_Rutherfordium,"Rf",163,267.122,-1,-1,-1,-1,-1,-1,-1},
  {105,4,6,TRANSITION_METAL,I18n::Message::AtomName_Dubnium,"Db",163,268.126,-1,-1,-1,-1,-1,-1,-1},
  {106,5,6,TRANSITION_METAL,I18n::Message::AtomName_Seaborgium,"Sg",163,269.128,-1,-1,-1,-1,-1,-1,-1},
  {107,6,6,TRANSITION_METAL,I18n::Message::AtomName_Bohrium,"Bh",163,270.133,-1,-1,-1,-1,-1,-1,-1},
  {108,7,6,TRANSITION_METAL,I18n::Message::AtomName_Hassium,"Hs",169,269.134,-1,-1,-1,-1,-1,-1,-1},
  {109,8,6,UNKNOWN,I18n::Message::AtomName_Meitnerium,"Mt",169,277.154,-1,-1,-1,-1,-1,-1,-1},
  {110,9,6,UNKNOWN,I18n::Message::AtomName_Darmstadtium,"Ds",171,282.166,-1,-1,-1,-1,-1,-1,-1},
  {111,10,6,UNKNOWN,I18n::Message::AtomName_Roentgenium,"Rg",171,282.169,-1,-1,-1,-1,-1,-1,-1},
  {112,11,6,POST_TRANSITION_METAL,I18n::Message::AtomName_Copernicium,"Cn",173,286.179,-1,-1,-1,-1,-1,-1,-1},
  {113,12,6,UNKNOWN,I18n::Message::AtomName_Nihonium,"Nh",173,286.182,-1,-1,-1,-1,-1,-1,-1},
  {114,13,6,UNKNOWN,I18n::Message::AtomName_Flerovium,"Fl",175,290.192,-1,-1,-1,-1,-1,-1,-1},
  {115,14,6,UNKNOWN,I18n::Message::AtomName_Moscovium,"Mc",174,290.196,-1,-1,-1,-1,-1,-1,-1},
  {116,15,6,UNKNOWN,I18n::Message::AtomName_Livermorium,"Lv",177,293.205,-1,-1,-1,-1,-1,-1,-1},
  {117,16,6,UNKNOWN,I18n::Message::AtomName_Tennessine,"Ts",177,294.211,-1,-1,-1,-1,-1,-1,-1},
  {118,17,6,NOBLE_GAS,I18n::Message::AtomName_Oganesson,"Og",176,295.216,-1,-1,-1,-1,-1,-1,-1},
};

constexpr size_t k_atomsCount = sizeof(atomsdefs) / sizeof(atomsdefs[0]);

#endif
