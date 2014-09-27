//
//  DeviceList.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import Cocoa
import CoreBluetooth

class DeviceList: NSObject, NSTableViewDataSource, NSTableViewDelegate {
  
  let manager: DeviceManager
  let devices: NSTableView
  var names = [String]()
  var delegate: DeviceListDelegate?
  
  init(devices: NSTableView, manager: DeviceManager) {
    self.devices = devices
    self.manager = manager
    super.init()
    devices.setDataSource(self)
    devices.selectionHighlightStyle = .Regular
    devices.target = self
    devices.doubleAction = "selectDevice:"
  }
  
  func scan() {
    manager.findDevices() {
      device in
      if !contains(self.names, device.name) {
        self.names.append(device.name)
        self.devices.reloadData()
      }
    }
  }
  
  func numberOfRowsInTableView(tableView: NSTableView!) -> Int {
    return names.count
  }
  
  func tableView(tableView: NSTableView!, objectValueForTableColumn tableColumn: NSTableColumn!, row: Int) -> AnyObject! {
    return names[row]
  }
  
  func selectDevice(id: AnyObject) {
    if let device = manager.devices[names[devices.clickedRow]] {
      delegate?.onDeviceConnect(device)
    }
  }
}