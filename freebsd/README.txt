This directory holds a directory for each FreeBSD distribution
to support. In the case arch makes a different a different directory
will be created.

However, until now, no difference has been found so the directories
10-x86-amd64 and 9-x86-amd64 also provides support for i386 arch.

   FreeBSD 10.X amd64:  ln -s 10-x86-amd64 arch
   FreeBSD 10.X i386:   ln -s 10-x86-amd64 arch

   FreeBSD 9.X amd64:   ln -s 9-x86-amd64 arch
   FreeBSD 9.X i386:    ln -s 9-x86-amd64 arch

..then call build packages (after having everything compiled ok).

   >> ./buildpackages.sh

