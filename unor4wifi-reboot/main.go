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
	"errors"
	"fmt"
	"os"

	"github.com/karalabe/hid"
)

// The following example was adapted from the HIDAPI documentation to
// demonstrate use of the hid package to communicate with a simple device.
func reboot_unor4() error {
	b := make([]byte, 65)

	// Open the device using the VID and PID.
	info := hid.Enumerate(0x2341, 0x1002)

	var d *hid.Device
	var err error

	if len(info) == 0 {
		fmt.Printf("Cannot enumerate, try direct open\n")
		d, err = hid.Open(0x2341, 0x1002)
		if err != nil {
			fmt.Printf("No board connected\n")
			return errors.New("No board connected")
		}
	} else {
		d, _ = info[0].Open()
	}

	// Reboot
	b[0] = 0
	b[1] = 0xAA
	if _, err := d.SendFeatureReport(b); err != nil {
		return err
	}

	err = d.Close()
	if err != nil {
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
