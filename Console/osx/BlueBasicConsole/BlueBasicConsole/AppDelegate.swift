//
//  AppDelegate.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Cocoa
import CoreBluetooth

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate, DeviceListDelegate {

  @IBOutlet weak var window: NSWindow!
  @IBOutlet weak var devicesView: NSTableView!
  @IBOutlet weak var statusField: NSTextField!
  @IBOutlet weak var consoleView: NSScrollView!
  
  let manager = DeviceManager()
  var console: Console!
  var devices: DeviceList!

  func applicationDidFinishLaunching(aNotification: NSNotification?) {
    
    console = Console(console: consoleView.contentView.documentView as NSTextView, status: statusField)
    devices = DeviceList(devices: devicesView, manager: manager)
    devices.delegate = self
    devices.scan()
  }
  
  func applicationShouldTerminateAfterLastWindowClosed(sender: NSApplication!) -> Bool {
    return true
  }
  
  func applicationWillTerminate(aNotification: NSNotification?) {
    console.disconnect()
  }
  
  func onDeviceConnect(device: Device) {
    console.connectTo(device)
  }
  
  func onDeviceDisconnect(device: Device) {
    console.disconnect()
  }
  
  
  @IBAction func fromDevice(sender: AnyObject) {
  }
  
  @IBAction func toDevice(sender: AnyObject) {
    if console.isConnected {
      var panel = NSOpenPanel()
      panel.canChooseFiles = true
      panel.title = "Select BASIC file to load onto device"
      if panel.runModal() == NSOKButton {
        Uploader(self.console!).upload(panel.URL!)
      }
    }
  }
  
  @IBAction func updateFirmware(sender: AnyObject) {
    if console.isUpgradable || console.isConnected {
      var panel = NSOpenPanel()
      panel.canChooseFiles = true
      panel.canChooseDirectories = false
      panel.allowsMultipleSelection = false
      panel.title = "Select new firmware for device"
      panel.allowedFileTypes = [ "bin" ]
      if panel.runModal() == NSOKButton {
        Firmware(self.console!).upgrade(panel.URL!)
      }
    }
  }
}

