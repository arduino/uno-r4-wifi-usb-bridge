// Copyright (c) 2019 Steven Stallion <sstallion@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

package main

import (
	"fmt"
	"os"

	"github.com/sstallion/go-hid"
)

// The following example was adapted from the HIDAPI documentation to
// demonstrate use of the hid package to communicate with a simple device.
func reboot_unor4() error {
	b := make([]byte, 65)

	// Initialize the hid package.
	if err := hid.Init(); err != nil {
		return err
	}

	// Open the device using the VID and PID.
	d, err := hid.OpenFirst(0x2341, 0x1002)
	if err != nil {
		return err
	}

	// Read the Manufacturer String.
	s, err := d.GetMfrStr()
	if err != nil {
		return err
	}
	fmt.Printf("Manufacturer String: %s\n", s)

	// Read the Product String.
	s, err = d.GetProductStr()
	if err != nil {
		return err
	}
	fmt.Printf("Product String: %s\n", s)

	// Read the Serial Number String.
	s, err = d.GetSerialNbr()
	if err != nil {
		return err
	}
	fmt.Printf("Serial Number String: %s\n", s)

	// get version
	g := make([]byte, 65)
	if _, err := d.GetFeatureReport(g); err != nil {
		return err
	}
	fmt.Printf("FW version: %d.%d.%d\n", g[1], g[2], g[3])

	// Reboot
	b[0] = 0
	b[1] = 0xAA
	if _, err := d.SendFeatureReport(b); err != nil {
		return err
	}

	// Finalize the hid package.
	if err := hid.Exit(); err != nil {
		return err
	}
	return nil
}

// The following example demonstrates use of the Enumerate function to display
// device information for all HID devices attached to the system.
func main() {
	err := reboot_unor4()
	if err == nil {
		os.Exit(0)
	} else {
		os.Exit(1)
	}
}
