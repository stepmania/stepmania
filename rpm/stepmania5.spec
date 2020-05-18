Name:       stepmania5
Version:    0
Release:    12
Summary:    A dance and rhythm game
License:    GPL
BuildRequires: gcc-c++ cmake libmad-devel libvorbis-devel libbz2-devel libX11-devel libjpeg62-devel libXtst-devel libpulse-devel alsa-devel libva-devel glew-devel libXrandr-devel
Requires:      libmad0 libvorbis0 libvorbisfile3 libbz2-1 libX11-6 libjpeg62 libXtst6 libpulse0 libasound2 libva2 libGLEW2_2 libXrandr2

%description
StepMania is a free dance and rhythm game. It features 3D graphics, keyboard and "dance pad" support, and an editor for creating your own steps.

%prep
#---------------------
#--- Checkout repo ---
#---------------------
rm -rf %{_builddir}/stepmania
git clone ~/git/stepmania %{_builddir}/stepmania || git clone https://github.com/stepmania/stepmania.git %{_builddir}/stepmania
#------------------------
#--- Start menu entry ---
#------------------------
install %{_builddir}/stepmania/rpm/stepmania5.desktop %{buildroot}/usr/share/applications/stepmania5.desktop

%build
cd %{_builddir}/stepmania
mkdir build-rpm
cd build-rpm
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{buildroot}/opt ..
make -j5

%install
cd %{_builddir}/stepmania/build-rpm
make install

%files
/opt/stepmania-5.1
#/usr/share/applications/stepmania5.desktop

%changelog
# let's skip this for now
