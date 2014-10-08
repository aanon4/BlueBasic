//
//  DeviceManager.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class DeviceManager: NSObject, CBCentralManagerDelegate {
  
  let manager: CBCentralManager!
  var scanning = false
  var devices = [NSUUID: Device]()
  var connectCallbacks = OneTimeCallbacks<Bool>()
  var disconnectCallbacks = OneTimeCallbacks<Bool>()
  var findCallbacks = Callbacks<Device>()
  
  override init() {
    super.init()
    manager = CBCentralManager(delegate: self, queue: dispatch_get_main_queue())
  }
  
  func findDevices(onNewDevice: NewDeviceFoundHandler) {
    findCallbacks.append(onNewDevice)
    startScan();
  }
  
  func startScan() {
    if !scanning {
      scanning = true
      if manager.state == .PoweredOn {
        scan()
      }
    }
  }
  
  func stopStan() {
    if scanning {
      scanning = false
      manager.stopScan()
      findCallbacks.removeAll()
    }
  }

  func connect(device: Device, onConnected: CompletionHandler?) {
    if !device.isConnected {
      connectCallbacks.append(onConnected)
      manager.connectPeripheral(device.peripheral, options: nil)
    } else {
      onConnected?(true)
    }
  }
  
  func disconnect(device: Device, onDisconnect: CompletionHandler?) {
    if device.isConnected {
      disconnectCallbacks.append(onDisconnect)
      manager.cancelPeripheralConnection(device.peripheral)
    } else {
      onDisconnect?(true)
    }
  }
  
  func centralManagerDidUpdateState(central: CBCentralManager!) {
    switch (manager.state) {
    case .PoweredOn:
      if scanning {
        scan()
      }
    default:
      break
    }
  }

  func scan() {
    manager.scanForPeripheralsWithServices(nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])
  }

  
  func centralManager(central: CBCentralManager!, didDiscoverPeripheral peripheral: CBPeripheral!, advertisementData: [NSObject : AnyObject]!, RSSI: NSNumber!) {
    let name = deviceName(peripheral)
    if let device = devices[peripheral.identifier!] {
      if (RSSI.integerValue <= 0) {
        device.rssi = RSSI.integerValue
      }
    } else {
      let device = Device(peripheral: peripheral, rssi: RSSI.integerValue, manager: self)
      devices[peripheral.identifier] = device
    }
    findCallbacks.call(devices[peripheral.identifier]!)
  }
  
  func centralManager(central: CBCentralManager!, didConnectPeripheral peripheral: CBPeripheral!) {
    if let device = devices[peripheral.identifier] {
      device.isConnected = true
      connectCallbacks.call(true)
    }
  }

  func centralManager(central: CBCentralManager!, didFailToConnectPeripheral peripheral: CBPeripheral!, error: NSError!) {
    if let device = devices[peripheral.identifier] {
      connectCallbacks.call(false)
    }
  }
  
  func centralManager(central: CBCentralManager!, didDisconnectPeripheral peripheral: CBPeripheral!, error: NSError!) {
    if let device = devices[peripheral.identifier] {
      device.isConnected = false
      disconnectCallbacks.call(error == nil)
    }
  }
  
  func deviceName(peripheral: CBPeripheral) -> String {
    if let n = peripheral.name {
      return n
    } else {
      return "(null)"
    }
  }
}
