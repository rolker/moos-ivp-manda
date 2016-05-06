#!/bin/bash 
#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
TIME_WARP=10
JUST_MAKE="no"
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [SWITCHES] [time_warp]   \n" $0
	printf "  --just_build, -j    \n" 
	printf "  --help, -h         \n" 
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
	JUST_MAKE="yes"
    else 
	printf "Bad Argument: %s \n" $ARGI
	exit 0
    fi
done

#-------------------------------------------------------
#  Part 2: Create the .moos and .bhv files. 
#-------------------------------------------------------
SCENARIO=3
# 1 = Summer Hydro 2015
# 2 = Strait of Georgia 
# 3 = Fairweather H12758 Offshore
if [ $SCENARIO = 1 ]; then
  #MOOS Stuff
  START_POS="x=705,y=-4379,speed=0,heading=180"
  LAT_ORIGIN=42.97373611
  LONG_ORIGIN=-70.7968875
  TIFF_FILE=SH_2015.tif
  PAN_X=814 #258
  PAN_Y=769 #927
  ZOOM=1.98 #0.53
  # Python Stuff
  OP_POLY="[(655,-4429),(550,-4813),(198,-4725),(300,-4353)]"
  OP_WKT="POLYGON_((655_-4429,550_-4813,198_-4725,300_-4353))"
  BATHY_GRID="'../path_planning/terrain/SH15_Surface.tif'"
  X_OFFSET=353408.656
  Y_OFFSET=6083.832+4753335.914
elif [ $SCENARIO = 2 ]; then
  #MOOS Stuff
  # START_POS="4082,9023"
  START_POS="4998,13971"
  LAT_ORIGIN=48.7188051
  LONG_ORIGIN=-122.8272363
  TIFF_FILE=H12322_8m_Color.tif
  PAN_X=72
  PAN_Y=-115
  ZOOM=0.86
  # Python Stuff
  # OP_POLY="[(4032,9073),(2838,12080),(5720,13208),(7249,10238)]"
  OP_POLY="[(4948,14031),(3916,14613),(5226,16027),(5579,15694),(5954,15515)]"
  BATHY_GRID="'../path_planning/terrain/H12322_8m_Depths.tiff'"
  X_OFFSET=512684.0
  Y_OFFSET=5396228.0
elif [ $SCENARIO = 3 ]; then
  #MOOS Stuff
  # START_POS="4082,9023"
  START_POS="x=-1625,y=4896,speed=0,heading=225"
  LAT_ORIGIN=55.1622067
  LONG_ORIGIN=-159.2832076
  TIFF_FILE=FA_Polygon1_Color.tif
  PAN_X=324
  PAN_Y=125
  ZOOM=0.13
  OP_WKT="POLYGON_((-1663_4653,-4729_-3130,-2758_-3624,-1090_-4660,396_421,964_3757))"
  # Python Stuff
  #OP_POLY="[(4948,14031),(3916,14613),(5226,16027),(5579,15694),(5954,15515)]"
  BATHY_GRID="'../path_planning/terrain/FA_Polygon1_Depths.tif'"
  X_OFFSET=-1029929.473
  Y_OFFSET=6385706.239
fi
# What is nsplug? Type "nsplug --help" or "nsplug --manual" 

nsplug sonar_sim.moos targ_sonar_sim.moos --path=../shared_plugins -f WARP=$TIME_WARP \
   LAT_ORIGIN=$LAT_ORIGIN   LONG_ORIGIN=$LONG_ORIGIN   START_POS=$START_POS \
   TIFF_FILE="$TIFF_FILE"  PAN_X=$PAN_X  PAN_Y=$PAN_Y  ZOOM=$ZOOM  OP_WKT="$OP_WKT"

# nsplug ~/code/asv-dev/utilities/python_moosapps/path_plan.py \
#   ~/code/asv-dev/utilities/python_moosapps/targ_path_plan.py -f \
#   OP_POLY=$OP_POLY  WARP=$TIME_WARP

nsplug ~/code/asv-dev/utilities/python_moosapps/sonar_simulator.py \
  ~/code/asv-dev/utilities/python_moosapps/targ_sonar_simulator.py -f \
  BATHY_GRID=$BATHY_GRID  X_OFFSET=$X_OFFSET \
  Y_OFFSET=$Y_OFFSET WARP=$TIME_WARP

# nsplug ~/code/asv-dev/utilities/python_moosapps/record_swath.py \
#   ~/code/asv-dev/utilities/python_moosapps/targ_record_swath.py -f \
#   WARP=$TIME_WARP

if [ ${JUST_MAKE} = "yes" ] ; then
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Launch the processes
#-------------------------------------------------------
# printf "Launching $VNAME1 MOOS Community (WARP=%s) \n" $TIME_WARP
# pAntler targ_$VNAME1.moos >& /dev/null &
# sleep .5
# printf "Launching $VNAME2 MOOS Community (WARP=%s) \n" $TIME_WARP
# pAntler targ_$VNAME2.moos >& /dev/null &
# sleep .5
# printf "Launching $VNAME_ASV MOOS Community (WARP=%s) \n" $TIME_WARP
# pAntler targ_asv.moos >& /dev/null &
# sleep .5
# printf "Launching $SNAME MOOS Community (WARP=%s) \n"  $TIME_WARP
# pAntler targ_shoreside.moos >& /dev/null &
# printf "Done \n"

# uMAC targ_shoreside.moos

# printf "Killing all processes ... \n"
# kill %1 %2 %3 %4
# printf "Done killing processes.   \n"


