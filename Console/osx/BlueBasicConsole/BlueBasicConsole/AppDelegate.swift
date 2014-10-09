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
  @IBOutlet weak var toDeviceMenu: NSMenuItem!
  @IBOutlet weak var upgradeMenu: NSMenuItem!
  
  let manager = DeviceManager()
  var console: Console!
  var devices: DeviceList!
  var autoUpgrade: AutoUpdateFirmware?

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
    upgradeMenu.enabled = false
    toDeviceMenu.enabled = false
    console.connectTo(device) {
      success in
      if success {
        self.toDeviceMenu.enabled = true
        self.autoUpgrade = AutoUpdateFirmware(console: self.console)
        self.autoUpgrade!.detectUpgrade() {
          needupgrade in
          if needupgrade {
            self.console.status = "Upgrade available"
            self.upgradeMenu.enabled = true
          } else {
            self.autoUpgrade = nil
          }
        }
      }
    }
  }
  
  func onDeviceDisconnect(device: Device) {
    upgradeMenu.enabled = false
    toDeviceMenu.enabled = false
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
  
  @IBAction func upgrade(sender: AnyObject) {
    if autoUpgrade != nil {
      autoUpgrade!.upgrade()
    }
  }
}

