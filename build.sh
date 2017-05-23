export USE_CCACHE=1
export CCACHE_DIR=$PWD/.ccache
if [ ! -d $PWD/.ccache ]; then
ccache -M 1G
fi
export ARCH=arm64

function get_make_command()
{
  echo command make
}
function make()
{
    local start_time=$(date +"%s")
    $(get_make_command) "$@"
    local ret=$?
    local end_time=$(date +"%s")
    local tdiff=$(($end_time-$start_time))
    local hours=$(($tdiff / 3600 ))
    local mins=$((($tdiff % 3600) / 60))
    local secs=$(($tdiff % 60))
    local ncolors=$(tput colors 2>/dev/null)
    if [ -n "$ncolors" ] && [ $ncolors -ge 8 ]; then
        color_failed=$'\E'"[0;31m"
        color_success=$'\E'"[0;32m"
        color_reset=$'\E'"[00m"
    else
        color_failed=""
        color_success=""
        color_reset=""
    fi
    echo
    if [ $ret -eq 0 ] ; then
        echo -n "${color_success}#### make completed successfully "
    else
        echo -n "${color_failed}#### make failed to build some targets "
    fi
    if [ $hours -gt 0 ] ; then
        printf "(%02g:%02g:%02g (hh:mm:ss))" $hours $mins $secs
    elif [ $mins -gt 0 ] ; then
        printf "(%02g:%02g (mm:ss))" $mins $secs
    elif [ $secs -gt 0 ] ; then
        printf "(%s seconds)" $secs
    fi
    echo " ####${color_reset}"
    echo
    return $ret
}


if [ -d ~/toolchain/aarch64-4.9/bin/ ]; then
	echo "Found toolchain: Setting up . . . "
else
	echo "Missing toolchain: Obtaining from Google . . ."
	git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9 -b master ~/toolchain/aarch64-4.9 
fi
export CROSS_COMPILE=~/toolchain/aarch64-4.9/bin/aarch64-linux-android-

make exynos7420-noblelte_usa_spr_defconfig > /dev/null 2>&1

while read -p "Start build with:`echo $'\n ' `1) $(nproc) cores?`echo $'\n ' `2) $(($(nproc)/2)) cores?`echo $'\n '`3) $(($(nproc)/4)) cores?`echo $'\n '`X|x to cancel auto build`echo $'\n>'`" corechoice
do
case "$corechoice" in
	1 )	
		echo "Building with $(nproc) cores"
		make -j$(nproc)
		break
		;;

        2 )
		echo "Building with $(($(nproc)/2)) cores"
		make -j$(($(nproc)/2))
                break
                ;;
        3 )
		echo "Building with $(($(nproc)/4)) core(s)"
		make -j$(($(nproc)/4))
		break
                ;;
        X|x )
                echo "Cancelling auto build"
                break
                ;;
        * )
                echo "Invalid: Try again"
                ;;

esac
done
