
# Parse command line arguments.
CleanBuild=dontclean
BuildConfiguration=none

for var in "$@"
do
    if [[ $var =~ ^(Debug|Release|wxDebug|wxRelease)$ ]]; then
        BuildConfiguration=$var;
    fi
    if [[ $var = clean ]]; then
        CleanBuild=clean;
    fi
done

# Exit if a valid build config wasn't specified.
if [[ ! $BuildConfiguration =~ ^(Debug|Release|wxDebug|wxRelease)$ ]]; then
    echo "Specify a build configuration: Debug, Release, wxDebug, wxRelease"
    echo "add 'clean' to clean that configuration"
    exit 2
fi

# Clean and exit.
if [[ $CleanBuild == clean ]]; then
    echo "$(tput setaf 5)==> Cleaning MyFramework$(tput sgr0)"
    rm -r build/$BuildConfiguration
    exit 1
fi

# Build bullet3. Clean must be done manually.
if [ ! -d "Libraries/bullet3/bin" ]; then
    echo "$(tput setaf 5)==> Building Bullet3... this could take a while$(tput sgr0)"
    pushd Libraries/bullet3/build3 > /dev/null
        ./premake4_linux64 gmake --double
        cd gmake
	    make
    popd > /dev/null
fi

# Build MyEngine.
let NumJobs=$(nproc)*2
echo "$(tput setaf 2)==> Building MyEngine (make -j$NumJobs)$(tput sgr0)"

if [ ! -d "build" ]; then
    mkdir build
fi

if [ ! -d build/$BuildConfiguration ]; then
    mkdir build/$BuildConfiguration
    pushd build/$BuildConfiguration > /dev/null
        cmake -DCMAKE_BUILD_TYPE=$BuildConfiguration ../..
    popd > /dev/null
fi

pushd build/$BuildConfiguration > /dev/null
    make -j$NumJobs
popd > /dev/null
