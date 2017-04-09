package main

import (
	"github.com/jurgen-kluft/xcode"
	"github.com/jurgen-kluft/xtime/package"
)

func main() {
	xcode.Generate(xfilesystem.GetPackage())
}
