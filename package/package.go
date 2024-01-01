package cfilesystem

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	ctime "github.com/jurgen-kluft/ctime/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
	cvmem "github.com/jurgen-kluft/cvmem/package"
)

// GetPackage returns the package object of 'cfilesystem'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	ccorepkg := ccore.GetPackage()
	cbasepkg := cbase.GetPackage()
	cvmempkg := cvmem.GetPackage()
	ctimepkg := ctime.GetPackage()

	// The main (cfilesystem) package
	mainpkg := denv.NewPackage("cfilesystem")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(ccorepkg)
	mainpkg.AddPackage(cbasepkg)
	mainpkg.AddPackage(cvmempkg)
	mainpkg.AddPackage(ctimepkg)

	// 'cfilesystem' library
	mainlib := denv.SetupDefaultCppLibProject("cfilesystem", "github.com\\jurgen-kluft\\cfilesystem")
	mainlib.Dependencies = append(mainlib.Dependencies, ccorepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, cvmempkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, ctimepkg.GetMainLib())

	// 'cfilesystem' unittest project
	maintest := denv.SetupDefaultCppTestProject("cfilesystem_test", "github.com\\jurgen-kluft\\cfilesystem")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, ccorepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cvmempkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, ctimepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
