package xfilesystem

import (
	"github.com/jurgen-kluft/ccode/denv"
	"github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xentry/package"
	"github.com/jurgen-kluft/xtime/package"
)

// GetPackage returns the package object of 'xfilesystem'
func GetPackage() *denv.Package {
	// Dependencies
	xunittestpkg := xunittest.GetPackage()
	xentrypkg := xentry.GetPackage()
	xbasepkg := xbase.GetPackage()
	xtimepkg := xtime.GetPackage()

	// The main (xfilesystem) package
	mainpkg := denv.NewPackage("xfilesystem")
	mainpkg.AddPackage(xunittestpkg)
	mainpkg.AddPackage(xentrypkg)
	mainpkg.AddPackage(xbasepkg)
	mainpkg.AddPackage(xtimepkg)

	// 'xfilesystem' library
	mainlib := denv.SetupDefaultCppLibProject("xfilesystem", "github.com\\jurgen-kluft\\xfilesystem")
	mainlib.Dependencies = append(mainlib.Dependencies, xbasepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, xtimepkg.GetMainLib())

	// 'xfilesystem' unittest project
	maintest := denv.SetupDefaultCppTestProject("xfilesystem_test", "github.com\\jurgen-kluft\\xfilesystem")
	maintest.Dependencies = append(maintest.Dependencies, xunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xentrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xtimepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
