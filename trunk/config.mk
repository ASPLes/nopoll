# optional nsis installation
# makensis              = c:/ARCHIV~1/NSIS/makensis.exe
export makensis		= "C:/Program Files (x86)/NSIS/makensis.exe"

# Flags to compile OPENSSL
# Uncomment the following lines, placing the libraries following the path
# schema provided to get support OpenSSL
export OPENSSL_FLAGS = -DENABLE_TLS_SUPPORT -Ic:/OpenSSL-Win64/include/
export OPENSSL_LIBS = c:/OpenSSL-Win64/bin/libeay32.dll c:/OpenSSL-Win64/bin/ssleay32.dll

# platform version prefix
# export version_prefix           = -MinGW32
export version_prefix    = -MinGW64

# platform bits 
# export platform_bits     = 32
export platform_bits     = 64

# force build with debug
# export show_debug        = -DSHOW_DEBUG_LOG
export show_debug        =
