
              -----------------------------------------
                          noPoll for Windows
               Advanced Software Production Line, S.L.
              -----------------------------------------

1. Introducion

   This package includes all pieces necessary to start developing
   WebSocket enabled applications and to test how noPoll toolkit works.

   This package have been tested to work on Microsoft Windows XP
   platform, but it shouldn't be any problem on the following windows
   platforms including 2000, 2003 and advance server editions.

   If you feel you have found something missing or something that
   could be improved, comments and suggestions, drop us an email:

      nopoll@lists.aspl.es

   Or use the noPoll development mailing list:

      http://lists.aspl.es/cgi-bin/mailman/listinfo/nopoll

   You'll find more information at:

      http://www.aspl.es/nopoll

   Enjoy the noPoll!

2. Looking around to test the library

   If you are interested in testing the library, ensure you select
   "testing applications" to be installed at the installation process.

   Then check the "bin/" directory found inside your selected
   installation directory.

   Inside that directory you'll find, among other things, the
   following programs:

       - nopoll-regression-client.exe, nopoll-regression-listener.exe ::
         
         These are the tools used to check the library function on
         each release. It is a regression test that checks several
         functions from the library and must execute and finish
         properly on your platform.

         Please report any wrong function found while using these
         regression test. If you can, attach a patch fixing the bug
         you've found.

	  In general, all development to check and improve the library
	  is done through these files (nopoll-regression-client.c and
	  nopoll-regression-listener.c).

3. Using the package as a support installation for your programs

   If you are trying to deliver WebSocket enabled programs using
   noPoll, you can bundle your installation with this package. The
   "bin/" directory is the only piece of insterest: it contains all
   dll files that your binaries will require at runtime.

4. Using the package to develop

   Inside the installation provided, the following directories
   contains required files to develop:

    - bin: dll files and data files. Your program being developed will
      require them. 

      You can modify your environment variable PATH to include the bin
      directory of your noPoll installation so your dinamic library
      loader can find them.

    - lib: import libraries for gcc and VS installations. They are
      provided either by the usual windows way (.lib) or the gcc way
      (.dll.a) through the mingw environment.

    - include: this directory contains vortex headers (and all
      required headers that the vortex depends on).

5. But, how do I ....

   Use the mailing list to reach us if you find that some question is
   not properly answered on this readme file. 

   You have contact information at the top of this file. Feel free to
   drop us an email!

   noPOll is commercially supported. In the case you want professional
   support see http://www.aspl.es/nopoll/professional.html

6. That's all

   We hope you find useful this library, try to keep us updated if you
   develop new products, profiles, etc. We will appreciate that,
   making your produt/project to appear at the noPoll main site.

-- 
Advanced Software Production Line, S.L.
Av. Juan Carlos I, Nº13, 2ºC
Alcalá de Henares 28806 Madrid

Francis Brosnan Blázquez
<francis@aspl.es>
2017/06/09
