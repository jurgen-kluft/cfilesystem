package main

import (
	"github.com/jurgen-kluft/xcode"
	"github.com/jurgen-kluft/xfilesystem/package"
)

func main() {
	xcode.Init()
	xcode.Generate(xfilesystem.GetPackage())
}
