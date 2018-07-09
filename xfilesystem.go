package main

import (
	"github.com/jurgen-kluft/xcode"
	"github.com/jurgen-kluft/xfilesystem/package"
)

func main() {
	xcode.Generate(xfilesystem.GetPackage())
}
