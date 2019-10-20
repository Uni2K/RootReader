#!/bin/bash

#rm read
# -rpath option might be necessary with some ROOT installations
# -lSpectrum option might be necessary with some ROOT installations
#g++ geometry.C read.C analysis.C main.C -rpath ${ROOTSYS}/lib `root-config --libs --cflags` -lSpectrum -o read
#g++ geometry.C read.C analysis.C main.C `root-config --libs --cflags` -lSpectrum -o read

showInformation() {
    echo "  _____             _   _    _      _                 "
    echo " |  __ \           | | | |  | |    | |                "
    echo " | |__) |___   ___ | |_| |__| | ___| |_ __   ___ _ __ "
    echo " |  _  // _ \ / _ \| __|  __  |/ _ \ | '_ \ / _ \ '__|"
    echo " | | \ \ (_) | (_) | |_| |  | |  __/ | |_) |  __/ |   "
    echo " |_|  \_\___/ \___/ \__|_|  |_|\___|_| .__/ \___|_|   "
    echo "                                     | |              "
    echo "                                     |_|              "

    echo "This tool helps you to start your prefered ROOT reading analysis."

    echo "Made by Jan Zimmermann in July/October 2019 (jan4995@gmail.com)"
    echo "-------------------------------------------------------"

}

checkDependencies() {

    PKG_OK=$(dpkg-query -W --showformat='${Status}\n' zenity | grep "ok installed")
    if [ "" == "$PKG_OK" ]; then
        echo Please enter the sudo password to install Zenity
        sudo apt-get install zenity
    else
        echo "Zenity is installed! "
    fi

    BUS_OK=$(dpkg-query -W --showformat='${Status}\n' dbus-x11 | grep "ok installed")
    if [ "" == "$BUS_OK" ]; then
        echo Please enter the sudo password to install DBUSx11
        sudo apt-get install dbus-x11
    else
        echo "dbus-x11 is installed! "
    fi
}

chooseOutFolder() {
    outFolder=$(zenity --file-selection --directory --title "Select outpu Folder (.bin Files)?")
    echo $outFolder
}

chooseInFolder() {
    inFolder=$(zenity --file-selection --directory --title "Select input Folder (.bin Files)?")
}

createInAndOutFolder() {
    if [ ! -d "$1" ]; then
        mkdir "$1"
    fi
    if [ ! -d "$2" ]; then
        mkdir "$2"
    fi
}

parseRunList() {
    dir_name=$0
    rl_file=$1

    # construct array. contains the elements separated by "_" delimiter in $dir_name
    IFS="_" read -r -a fields <<<"$dir_name"

    nfields=${#fields[@]}

    # get run number
    runNr=${fields[0]}

    # get pdgID, beam energy
    runParticle=${fields[1]}
    case $runParticle in
    muon[6])
        pdgID="-13"
        energy="6"
        ;;
    pion[1-6])
        pdgID="211"
        energy=$(echo $runParticle | cut -c 5)
        ;;
    e5)
        pdgID="-11"
        energy=5
        ;;
    *)
        echo "UNKNOWN particle description in run $runNr"
        ;;
    esac

    # get measurement position
    runMP=${fields[2]}
    case $runMP in
    pos[0-9] | pos[1][0-9]) # 0°/30° measurements, no xy-coordinates
        frontMP=$(echo $runMP | cut -c 4-)
        ;;
    scanBD) # 90° WOM scans, hardcoded $sideMP
        side_pos_x=${fields[3]}
        side_pos_y=${fields[4]}
        case $side_pos_y in
        0)
            case $side_pos_x in
            0) sideMP=18 ;; 6) sideMP=19 ;; 12) sideMP=20 ;; 18) sideMP=21 ;; 24) sideMP=22 ;;
            esac
            ;;
        1)
            case $side_pos_x in
            0) sideMP=23 ;; 6) sideMP=24 ;; 12) sideMP=25 ;; 18) sideMP=26 ;; 24) sideMP=27 ;;
            esac
            ;;
        2)
            case $side_pos_x in
            0) sideMP=28 ;; 6) sideMP=29 ;; 12) sideMP=30 ;; 18) sideMP=31 ;; 24) sideMP=32 ;;
            esac
            ;;
        3)
            case $side_pos_x in
            0) sideMP=33 ;; 6) sideMP=34 ;; 12) sideMP=35 ;; 18) sideMP=36 ;; 24) sideMP=37 ;;
            esac
            ;;
        esac
        ;;
    *)
        echo "UNKNOWN position description in run $runNr"
        ;;
    esac

    # get angle
    case $nfields in
    3)
        angle="0"
        ;;
    4)
        angle="0"
        ;;
    5)
        angle=$(echo "${fields[3]}" | cut -c 6-)
        ;;
    7)
        angle=$(echo "${fields[5]}" | cut -c 6-)
        ;;
    *)
        echo "UNKNOWN angele description in run $runNr"
        ;;

    esac

    ######################
    ## PRINT TO RUNLIST ##
    ######################
    # line_to_runlsit
    case $nfields in
    [3-5])
        line_to_runlsit="$runNr $dir_name $frontMP $pdgID $energy $angle"
        ;;
    7)
        line_to_runlsit="$runNr $dir_name $sideMP $pdgID $energy $angle $side_pos_x $side_pos_y"
        ;;
    esac

    echo "$line_to_runlsit"
    echo "$line_to_runlsit" >>"$rl_file"

}

compileRead() {
    echo "Compiling ReadFull..."
    g++ ./src/geometry.C ./src/read.C ./src/analysis.C ./src/main.C ./src/calib.C $(root-config --libs --cflags) -lSpectrum -o ./src/read
    echo "Compiling done!"
}

readFull() {
    if [ "$shouldCompile" = true ]; then
        compileRead
    fi
    runNr=$1
    readAll=false
    inFolder=$2
    outFolder=$3
    headerSize=$4
    saveFolder=$outFolder/Full/

    if [ "$runNr" = "a" ]; then
        readAll=true
    fi
    mkdir "$saveFolder"


    while read line; do
        [[ $line == \#* ]] && continue
        lineArr=($line)
        if [ "${lineArr[0]}" = "$runNr" ] || [ $readAll = true ]; then
            runName=${lineArr[1]}
            runDir=$saveFolder/$runName
            mkdir "$runDir"
            if [ ! -e runDir/$runName.list ]; then
                ls $inFolder/$runName | grep \.bin >$runDir/$runName.list
            fi

            #time $here/readFull $runDir/$runName.list $inFolder/$runName/ $runDir/out.root ${lineArr[0]} ${lineArr[2]} ${lineArr[3]} ${lineArr[4]} ${lineArr[5]} ${lineArr[6]}
            time ./src/read $runDir/$runName.list $inFolder/$runName/ $runDir/$runName.root $runName $headerSize "$isDC" "$dynamicBL" "$useCalibValues"

        fi
    done <./RootRunlist

}

compileMerger() {
    echo "Compiling Root Merger..."
    g++ ./src/mergeROOTFiles.C $(root-config --libs --cflags) -lSpectrum -o mergeROOTFiles
    echo "Compiling done!"
}

merger() {
    rootFileList=$1

    $src/mergeROOTFiles $rootFileList $2 $3
}

readFast() {
    if [ "$shouldCompile" = true ]; then
        compileRead
    fi
    compileMerger

    runNr=$1
    readAll=false
    inFolder=$2
    outFolder=$3
    headerSize=$4
    saveFolder=$outFolder/Fast/

    if [ "$runNr" = "a" ]; then
        readAll=true
    fi
    mkdir "$saveFolder"

    rootFileList=""

    time (
        while read line; do
            #  echo $line
            [[ $line == \#* ]] && continue #Wenn die Zeile leer ist wird "continue" ausgeführt
            lineArr=($line)
            if [ "${lineArr[0]}" = "$runNr" ] || [ $readAll = true ]; then #lineArr ist ein Array aus jeder Zeile, getrennt durch Leerzeichen. linearray[0] ist die Run NUMMER

                runName=${lineArr[1]}

                runDir=$saveFolder/$runName
                mkdir "$runDir"
                if [ ! -e $runDir/$runName.list ]; then
                    ls $inFolder/$runName | grep \.bin >$runDir/$runName.list #Durchsucht das RunName verzeichnis nach bins files und erstellt eine Runlist
                fi

                counter=0
                #Für jede Bin einzeln durchlaufen
                while read line; do
                    #Create RunList for every file
                    echo $line >$runDir/$counter.list
                    mkdir $runDir/$counter

                    ./src/read $runDir/$counter.list $inFolder/$runName/ $runDir/$counter/out.root $runName $headerSize "$isDC" "$dynamicBL" "$useCalibValues"

                    runDirRelative="${runDir//$here/}"
                    rootTreeFilePath=".$runDirRelative/$counter/out.root/T"
                    rootFileList="${rootFileList}||$rootTreeFilePath"

                    echo "$rootFileList"

                    counter=$((counter + 1))

                done <$runDir/$runName.list

                merger $rootFileList $runDir $runName
                #rm $runDir/*.list
                rm -rf $runDir/*/
                find $runDir -name "*.list" -type f -delete



            fi

        done \
            <./RootRunlist

        #reads lines of "runslist"

    )

}

readRoot() {
    readMode=$1  #0= FULL #1=FAST
    runNumber=$2 #a=ALL
    inFolder=$3
    outFolder=$4
    headerSize=$5
    case $readMode in
    0)
        readFull $runNumber $inFolder $outFolder $headerSize
        ;;
    1)
        readFast $runNumber $inFolder $outFolder $headerSize
        ;;
    esac

}

saveConfig() {
    rm config.txt 2>/dev/null
    destdir=config.txt
    echo "$inFolder" >>"$destdir"
    echo "$outFolder" >>"$destdir"
    echo "$readMode" >>"$destdir"
    echo "$runNumber" >>"$destdir"
    echo "$headerSize" >>"$destdir"
    echo "$isDC" >>"$destdir"
    echo "$dynamicBL" >>"$destdir"
    echo "$useCalibValues" >>"$destdir"

    echo "config saved!"

}

loadConfig() {
    #destdir=config.txt
    #$inFolder=$( sed -n '1 p' config.txt )

    inFolder=$(awk 'NR == 1' config.txt)
    outFolder=$(awk 'NR == 2' config.txt)
    readMode=$(awk 'NR == 3' config.txt)
    runNumber=$(awk 'NR == 4' config.txt)
    headerSize=$(awk 'NR == 5' config.txt)
    isDC=$(awk 'NR == 6' config.txt)
    dynamicBL=$(awk 'NR == 7' config.txt)
    useCalibValues=$(awk 'NR == 8' config.txt)

    echo "config loaded!"
}

start() {
    echo "   _____ _             _   "
    echo "  / ____| |           | |  "
    echo " | (___ | |_ __ _ _ __| |_ "
    echo "  \___ \| __/ _\` | '__| __|"
    echo "  ____) | || (_| | |  | |_ "
    echo " |_____/ \__\__,_|_|   \__|"
    echo "                           "
    echo "                           "
    echo "--------------------------------PARAMETER--------------------------------"

    echo "Input data folder: $inFolder"
    echo "Output data folder: $outFolder"
    echo "Compiles the scripts: $shouldCompile"
    echo "RunMode: $runMode ($readMode)"
    echo "RunNumber: $runNumber"
    echo "Use Existing RunList: $useExistingRunList"
    echo "Header Size: $headerSize"
    echo "---------------------------------------------"

    createInAndOutFolder $inFolder $outFolder

    if [ "$useExistingRunList" = false ]; then
        echo "Creating a runlist for all input folder Files... (Name: RootRunlist)"
        export -f parseRunList
        runlist="RootRunlist"
        rm "$runlist"
        ls $inFolder | sort -n | xargs -n 1 bash -c "parseRunList $runlist"

        #Check if readList exists
        if [ -f $runList ]; then
            echo "Runlist created successfully!"
        else
            echo "Runlist creation failed!"
        fi
    fi

    readRoot $readMode $runNumber $inFolder $outFolder $headerSize
    echo "---------------------------------------------"
    echo "  ____    ___   _   _  _____ "
    echo " |  _ \  / _ \ | \ | || ____|"
    echo " | | | || | | ||  \| ||  _|  "
    echo " | |_| || |_| || |\  || |___ "
    echo " |____/  \___/ |_| \_||_____|"
    echo "                             "
    echo "---------------------------------------------"
}

# ██████╗ ██████╗ ██████╗ ███████╗
#██╔════╝██╔═══██╗██╔══██╗██╔════╝
#██║     ██║   ██║██║  ██║█████╗
#██║     ██║   ██║██║  ██║██╔══╝
#╚██████╗╚██████╔╝██████╔╝███████╗
# ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝

showInformation
checkDependencies
here=$(pwd) 
src=$here/src/
inFolder=$here/data
outFolder=$here/runs
shouldCompile=true
runMode="Full"
runNumber=21
readMode=0
headerSize=a
useExistingRunList=false
useCalibValues="0"
dynamicBL="1"
isDC="0"

echo "-------------------------------------------------------"

echo "Select an option!"

while true; do
    echo "-------------------------------------------------------"
    echo "-------------------------------------------------------"
    echo "-------------------------------------------------------"

    select yn in "Start" "RunMode ($runMode)" "RunNumber ($runNumber)" "Change Input Folder ($inFolder)" "Change Output Folder ($outFolder)" "Compile Readscripts ($shouldCompile)" "Binary Headersize ($headerSize)" "Calibration/Baseline" "Use existing RunList ($useExistingRunList)" "Load Config" "Save Config" "Informations"; do
        case $yn in

        "Start")
            printf "\033c" #Clean Screen
            start
            break 2
            ;;

        \
            "Calibration/Baseline")
            while true; do
                select yn in "Is Dark Count Measurement ($isDC)" "Dynamic Baseline ($dynamicBL)" "Use Constant Calibration Values ($useCalibValues)" "<- Back"; do
                    case $yn in

                    \
                        "<- Back")
                        break 2
                        ;;

                    "Is Dark Count Measurement ($isDC)")
                        if [ "$isDC" = "0" ]; then
                            isDC="1"
                        else
                            isDC="0"
                        fi
                        break
                        ;;
                    "Dynamic Baseline ($dynamicBL)")
                        if [ "$dynamicBL" = "0" ]; then
                            dynamicBL="1"
                        else
                            dynamicBL="0"
                        fi
                        break
                        ;;
                    "Use Constant Calibration Values ($useCalibValues)")
                        if [ "$useCalibValues" = "0" ]; then
                            useCalibValues="1"
                        else
                            useCalibValues="0"
                        fi
                        break
                        ;;
                    esac
                done
            done

            break
            ;;

        \
            "RunNumber ($runNumber)")
            echo "Enter a runNumber (ALL=a, Example: 22)"
            read runNumber
            break
            ;;

        "Change Input Folder ($inFolder)")
            chooseInFolder
            break
            ;;
        "Change Output Folder ($outFolder)")
            chooseOutFolder
            break
            ;;
        "Compile Readscripts ($shouldCompile)")
            if [ "$shouldCompile" = false ]; then
                shouldCompile=true
            else
                shouldCompile=false
            fi

            break
            ;;
        "RunMode ($runMode)")
            if [ "$runMode" = "Full" ]; then
                runMode="Fast"
                readMode=1
            else
                runMode="Full"
                readMode=0
            fi
            break
            ;;

        "Use existing RunList ($useExistingRunList)")
            if [ "$useExistingRunList" = false ]; then
                useExistingRunList=true

            else
                useExistingRunList=false
            fi
            break
            ;;
        "Binary Headersize ($headerSize)")
            echo "Enter the headersize of the binary files (Depends on the WC version)"
            echo "Version: <2.8.14: 327,  ==2.8.14: 328,  >2.8.14: 403   -> a=automatic"

            read headerSize
            break
            ;;

        \
            "Load Config")
            loadConfig
            break
            ;;
        "Save Config")
            saveConfig
            break
            ;;
        "Informations")
            echo "-------------------------------------------------------"

            echo "RunMode Informations: "
            echo "Full: High memory usage but all histogramms (1 Root file, fast at small datasets)"
            echo "Fast: Low memory usage but broken histogramms (Multiple merged Root file, slow at small datasets)"
            echo "RunList: Has to be in the same directory as this script, named: RootRunlist, Content like: 22 22_muon6_pos4 4 -13 6 0 AB per Line"
            echo "-------------------------------------------------------"
            echo "Header: The header size (open binary file with editor and count chars at the top)"
            echo "changes from version to version. And also from different WC channel sizes"
            echo "There are some changes that have to be made in the read.C scripts to use a new header size"
            echo "Changes like reading new data in the event structure or reading a dummy byte!"
            echo "Just put headersize on 'a' to let the system detect the headersize by reading the printable chars at the .bin beginning"
            echo "Maybe helping: https://owncloud.lal.in2p3.fr/index.php/s/3c1804f473fc1b2d3de454a23329b4a7"
            echo "-------------------------------------------------------"

            break
            ;;

        esac

    done

done

# sudo apt-get install zenity
#FILE=$(dialog --title "Delete a file" --stdout --title "Please choose a file to delete" --fselect /tmp/ 14 48)
