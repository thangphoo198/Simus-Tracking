#!/bin/sh

export Start_at=`date +%H:%M:%S`
export PROJECT_ROOT=$PWD
export curtitle="cmd:"$0
export ret="0"
export def_proj="EC200UCN_AA"
export buildtype_src=$1

doneseg(){
	echo " "
	date +%Y/%m/%d 
	echo "START TIME: $Start_at"
	echo "END TIME: `date +%H:%M:%S`"
	exit $ret
}

helpinfo(){
	echo "********************************************************************"
	echo "!!!!!!!!!!!      Please input right build option      !!!!!!!!!!!!!!"
	echo "********************************************************************"
	exit $ret
}

buildinfo(){
	echo "For your information:"
	echo "Usage: $0 r/new Project Version [VOLTE] [DSIM] [debug/release]"
	echo "       $0 clean"
	echo "       $0 h/-h"
	echo "Example:"
	echo "       $0 new $def_proj your_version_label"
	echo "       $0 new $def_proj your_version_label VOLTE SINGLESIM debug"
	echo "       $0 new $def_proj your_version_label NOVOLTE DOUBLESIM release"
	echo " "
	echo "Note: Optional parameter: [VOLTE] [DSIM]"
	echo "      The previous optional parameter is required for the latter optional parameter."
	echo "      For example, If you need DOUBLESIM with NOVOLTE, please set [VOLTE] as NOVOLTE and [DSIM] as DOUBLESIM."
	echo " "
	echo "      [VOLTE] option is for VoLTE feature, and NOVOLTE is default."
	echo "      If you want volte, you can ignore [VOLTE] with SINGLESIM or set it as VOLTE. If you want no volte, please set [VOLTE] as NOVOLTE!"
	echo "      [DSIM] option is for single or double sim card. and SINGLESIM is default."
	echo "      If you want single sim, you can ignore [DSIM] or set it as SINGLESIM. If you want double sim, please set [DSIM] as DOUBLESIM!"
	echo " "
	echo "you can get supported [Project] at the directory components\ql-config\build,"
	echo "the name of folders are supported [Project], as below:"
	echo "components/ql-config/build/"
	exit $ret
}

buildstart(){
	optionhelp=0
	if test $buildoption = "clean"
	then
		rm -rf out
	    echo "cleaning done"
		doneseg
	elif test $buildoption = "r"
	then
		echo " "
		#buildseg
	elif test $buildoption = "new"
	then
		echo "cleaning..."
		rm -rf out 
		echo "cleaning done"
		#buildseg
	else
		optionhelp=1
	    doneseg
	fi
}

PATH=$PROJECT_ROOT/prebuilts/linux/bin:$PATH
PATH=$PROJECT_ROOT/prebuilts/linux/cmake/bin:$PATH
PATH=$PROJECT_ROOT/prebuilts/linux/gcc-arm-none-eabi/bin:$PATH
PATH=$PROJECT_ROOT/prebuilts/linux/gcc-mips-rda-elf/bin:$PATH
PATH=$PROJECT_ROOT/prebuilts/linux/gcc-rv32-elf/bin:$PATH
PATH=$PROJECT_ROOT/prebuilts/linux/nanopb:$PATH
PATH=$PROJECT_ROOT/tools:$PATH
PATH=$PROJECT_ROOT/tools/linux:$PATH


if test "$quec_proj" = ""; then
	quec_proj=$quec_def_proj
fi
	
if test "$build_project" = ""; then
	build_project=$def_proj
fi
	
if test "$buildtype_src" = ""; then
	buildoption=new
elif test "$buildtype_src" = "r"; then
	buildoption=r
elif test "$buildtype_src" = "new"; then
	buildoption=new
elif test "$buildtype_src" = "clean"; then
	buildoption=clean
elif test "$buildtype_src" = "c"; then
	buildoption=clean
elif test "$buildtype_src" = "h"; then
	buildoption=h
elif test "$buildtype_src" = "-h"; then
	buildoption=h
elif test "$buildtype_src" = "help"; then
	buildoption=h
elif test "$buildtype_src" = "/h"; then
	buildoption=h
elif test "$buildtype_src" = "/?"; then
	buildoption=h
elif test "$buildtype_src" = "?"; then
	buildoption=h
else 
	echo "!!!unknown build type: $buildoption, should be r/new/clean/h/-h!!!"
	helpinfo	
fi

if test "$buildoption" = "h"; then
	buildinfo
fi

if test "$buildoption" = "clean"; then
	buildstart
fi 

export ql_buildproj=$2
if test "$ql_buildproj" = ""; then
	ql_buildproj=$def_proj
fi

if test "$3" = ""; then
	echo "we need your version label..."
	helpinfo
fi

export volte_enable=$4
export quec_dsim=$5

echo $volte_enable $quec_dsim

export ql_dsim_cfg="n"
export modemdir="cat1_UIS8915DM_BB_RF_SS_cus"
export partitionfile="components/hal/config/8910/partinfo_8910_8m_opencpu.json"
if test "${quec_dsim}" != "DOUBLESIM"; then
	quec_dsim="SINGLESIM"
fi

if test "${quec_dsim}" = "DOUBLESIM"; then
	ql_dsim_cfg="y"
	modemdir="cat1_UIS8915DM_BB_RF_DS_cus"
	partitionfile="components/hal/config/8910/partinfo_8910_8m_opencpu_ds.json"
fi
export prepack_json_path="components/ql-config/download/prepack/ql_prepack.json"
export ap_ram_offset="0xC00000"
export ap_ram_size="0x400000"
export ims_delta_nv="y"
export quec_ims_feature="y"
if test "$volte_enable" != "VOLTE"; then
	volte_enable="NOVOLTE"
fi

if test "$volte_enable" = "NOVOLTE"; then
	quec_ims_feature="n"
	ap_ram_offset="0x980000"
	ap_ram_size="0x680000"
	ims_delta_nv="n"
	modemdir="cat1_UIS8915DM_BB_RF_SS_NoVolte_cus"
	partitionfile="components/hal/config/8910/partinfo_8910_8m_opencpu_novolte.json"
	if test "${quec_dsim}" = "DOUBLESIM"; then
		modemdir="cat1_UIS8915DM_BB_RF_DS_NoVolte_cus"
	fi
fi

echo $PATH

export buildver=$3
buildver=${buildver^^}

export outputtype=$6
if test "$outputtype" != "debug";then
	outputtype="release"
fi

export _ccsdk_build="ON"
export _OPEN_="OPEN_CPU"
export BUILD_TARGET="8915DM_cat1_open"
export BUILD_RELEASE_TYPE=$outputtype
export ql_app_ver=${buildver}_APP

if [ ! -d "${PROJECT_ROOT}/components/ql-config/build/${ql_buildproj}/${BUILD_TARGET}" ]; then
	echo " "
	echo "********************        ERROR        ***********************"
	echo "your target.config is not exist:"
	echo "    ${PROJECT_ROOT}/components/ql-config/build/${ql_buildproj}/${BUILD_TARGET}"
	echo "****************************************************************"
	echo " "
	ret=1
	buildinfo
fi

export KCONFIG_CONFIG=$PROJECT_ROOT/components/ql-config/build/${ql_buildproj}/${BUILD_TARGET}/target.config
export target_out_dir=out/${BUILD_TARGET}_${BUILD_RELEASE_TYPE}
export PROJECT_OUT=${PROJECT_ROOT}/$target_out_dir

region=${ql_buildproj:6:2}
if test "$region" = "CN"; then
    export IMS_DELTA_NV_ROOT=components/ql-kernel/modem/ims/cn
elif test "$region" = "EU"; then
    export IMS_DELTA_NV_ROOT=components/ql-kernel/modem/ims/eu
elif test "$region" = "LA"; then
    export IMS_DELTA_NV_ROOT=components/ql-kernel/modem/ims/la
else
    export IMS_DELTA_NV_ROOT=components/ql-kernel/modem/ims/xx
fi
echo ${IMS_DELTA_NV_ROOT}

if test -f prebuilts/modem/8910/${modemdir}/nvitem/*.prj; then 
	rm -rf prebuilts/modem/8910/${modemdir}/nvitem/*.prj
fi

cp -r components/ql-config/build/${ql_buildproj}/nvitem/delta_base.nv components/nvitem/8910/default/deltanv/
cp -r components/ql-config/build/${ql_buildproj}/nvitem/delta_ims.nv components/nvitem/8910/default/deltanv/
cp -r components/ql-config/build/${ql_buildproj}/nvitem/delta_simcard2.nv components/nvitem/8910/default/deltanv/


if test "$volte_enable" = "NOVOLTE"; then
	if test "${quec_dsim}" = "DOUBLESIM"; then
		cp -r components/ql-config/build/${ql_buildproj}/nvitem/nvitem_modem_novolte_ds.prj prebuilts/modem/8910/${modemdir}/nvitem/nvitem_modem.prj
	else
		cp -r components/ql-config/build/${ql_buildproj}/nvitem/nvitem_modem_novolte.prj prebuilts/modem/8910/${modemdir}/nvitem/nvitem_modem.prj
	fi
else
	if test "${quec_dsim}" = "DOUBLESIM"; then
		cp -r components/ql-config/build/${ql_buildproj}/nvitem/nvitem_modem_ds.prj prebuilts/modem/8910/${modemdir}/nvitem/nvitem_modem.prj
	else
		cp -r components/ql-config/build/${ql_buildproj}/nvitem/nvitem_modem.prj prebuilts/modem/8910/${modemdir}/nvitem/nvitem_modem.prj
	fi
fi

#cp -r components/ql-config/build/${ql_buildproj}/quec_proj_config.h components/ql-kernel/inc
#cp -r components/ql-config/build/${ql_buildproj}/quec_proj_config_at.h components/ql-kernel/inc
cp -r components/ql-config/build/${ql_buildproj}/ql_libs/libql_at.a components/ql-kernel/libs

if [ -d "$ROJECT_OUT" ]; then
	rm -rf ../../target/$target_out/
fi
mkdir -p $PROJECT_OUT
sudo chmod -R 777 $PROJECT_OUT

if [ -d "${PROJECT_OUT}/include" ]; then
	rm -rf ${PROJECT_OUT}/include
fi

mkdir -p ${PROJECT_OUT}/include
sudo chmod -R 777 ${PROJECT_OUT}/include

cd out/${BUILD_TARGET}_${BUILD_RELEASE_TYPE}

sudo chmod -R 777 ../../prebuilts
sudo chmod -R 777 ../../out
sudo chmod -R 777 ../../out/${BUILD_TARGET}_${BUILD_RELEASE_TYPE}
sudo chmod -R 777 ../../tools
sudo chmod -R 777 ../../tools/linux
sudo chmod -R 777 ../../components/ql-config/build/${ql_buildproj}/${BUILD_TARGET}/

cmake ../.. -G Ninja

sudo chmod -R 777 ../../out/${BUILD_TARGET}_${BUILD_RELEASE_TYPE}

ninja

if [ $? != 0 ];then
	cd $PROJECT_ROOT
	echo ""
	echo ""
	echo "********************        ERROR        ***********************"
	echo "********************        ERROR        ***********************"
	echo "****************************************************************"
	echo "xxxxxxxxxxxxxxxxxx   build ended error   xxxxxxxxxxxxxxxxxxxxxxx"
	echo "****************************************************************"
	echo "****************************************************************"
	export ret=1
	doneseg
fi

echo ""
echo "********************        PASS         ***********************"
echo "**************   build ended successfully   ********************"
echo "********************        PASS         ***********************"

echo $ql_buildproj $buildver $BUILD_RELEASE_TYPE $volte_enable $quec_dsim

export version_path=../../target/${ql_buildproj}_${buildver}

if [ -d "${version_path}/" ]; then
	rm -rf ${version_path}/	
fi

mkdir -p ${version_path}/prepack/
mkdir -p ${version_path}/app/

sudo chmod -R 777 ../../target

cp -r ./hex/. $version_path/prepack/
mv ${version_path}/prepack/${BUILD_TARGET}.elf ${version_path}/
mv ${version_path}/prepack/${BUILD_TARGET}.map ${version_path}/
mv ${version_path}/prepack/${BUILD_TARGET}*${BUILD_RELEASE_TYPE}.pac ${version_path}/
mv ${version_path}/${BUILD_TARGET}*${BUILD_RELEASE_TYPE}.pac ${version_path}/${BUILD_TARGET}_${buildver}.pac

cp -r ./target.cmake ${version_path}/prepack/
cp -r ./hex/examples/*.* ${version_path}/app/

export ql_prepack_opt=N
if test -f ql_prepack.opt; then
	ql_prepack_opt=Y
fi

if test "$ql_prepack_opt" = "Y"; then
	dtools pacmerge --id APPIMG,PS --id PREPACK,NV ${version_path}/${BUILD_TARGET}_${buildver}.pac ${version_path}/app/${ql_app_ver}.pac ${version_path}/${BUILD_TARGET}_${buildver}_merge.pac
else
	dtools pacmerge --id APPIMG,PS ${version_path}/${BUILD_TARGET}_${buildver}.pac ${version_path}/app/${ql_app_ver}.pac ${version_path}/${BUILD_TARGET}_${buildver}_merge.pac
fi

python3 ../../tools/codesize.py --map ./hex/${BUILD_TARGET}.map
mv ./outlib.csv $version_path/prepack/
mv ./outobj.csv $version_path/prepack/
mv ./outsect.csv $version_path/prepack/
sudo chmod -R 777 $version_path

echo "build complete!!!"
