//
//  Device.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class Device: NSObject, CBPeripheralDelegate {
  
  let peripheral: CBPeripheral
  let manager: DeviceManager
  var serviceList = [CBUUID: DeviceService]()
  var isPopulated = false
  var serviceCallbacks = OneTimeCallbacks<[CBUUID: DeviceService]>()
  var characteristics = 0
  var delegate: DeviceDelegate?
  var readCallbacks = [CBUUID: OneTimeCallbacks<NSData?>]()
  
  var rssi: Int
  
  init (peripheral: CBPeripheral, rssi: Int, manager: DeviceManager) {
    self.peripheral = peripheral
    self.manager = manager
    self.rssi = rssi
    super.init()
    self.peripheral.delegate = self
  }
  
  func services(callback: ServicesFoundHandler) {
    if !isPopulated {
      serviceCallbacks.append(callback)
      if !isConnected {
        manager.connect(self) {
          success in
          if success {
            self.peripheral.discoverServices([])
          } else {
            self.serviceCallbacks.call(self.serviceList)
          }
        }
      } else {
        peripheral.discoverServices([])
      }
    } else {
      callback(serviceList)
    }
  }
  
  func connect(onConnected: CompletionHandler? = nil) {
    manager.connect(self) {
      success in
      if success {
        self.manager.disconnectCallbacks.append({
          ignore in
          var d = self.delegate
          d?.onDisconnect()
        })
      }
      onConnected?(success)
    }
  }
  
  func disconnect(onDisconnect: CompletionHandler? = nil) {
    manager.disconnect(self, onDisconnect)
  }
  
  var isConnected: Bool = false {
    didSet {
      if !isConnected && oldValue {
        serviceCallbacks.removeAll()
      } else if isConnected && !oldValue {
        isPopulated = false
        serviceList.removeAll(keepCapacity: false)
      }
    }
  }
  
  var name: String {
    get {
      return manager.deviceName(peripheral)
    }
  }
  
  var identifier: NSUUID {
    get {
      return peripheral.identifier;
    }
  }
  
  func write(data: NSData, characteristic: CBCharacteristic, type: CBCharacteristicWriteType) {
    if isConnected {
      peripheral.writeValue(data, forCharacteristic: characteristic, type: type)
    }
  }
  
  func notify(uuid: CBUUID, serviceUUID: CBUUID) {
    if isConnected {
      services() {
        list in
        if let characteristic = list[serviceUUID]?.characteristics[uuid] {
          self.peripheral.setNotifyValue(true, forCharacteristic: characteristic)
        }
      }
    }
  }
  
  func read(characteristic: CBCharacteristic, onRead: NSData? -> Void) {
    if isConnected {
      if readCallbacks[characteristic.UUID] == nil {
        readCallbacks[characteristic.UUID] = OneTimeCallbacks<NSData?>()
      }
      readCallbacks[characteristic.UUID]!.append(onRead)
      peripheral.readValueForCharacteristic(characteristic)
    } else {
      onRead(nil)
    }
  }
  
  func peripheral(peripheral: CBPeripheral!, didDiscoverServices error: NSError!) {
    if let services = peripheral.services as? [CBService] {
      characteristics = 0
      for service in services {
        serviceList[service.UUID] = DeviceService(device: self, service: service)
        peripheral.discoverCharacteristics([], forService: service)
        characteristics++
      }
    }
  }
  
  func peripheral(peripheral: CBPeripheral!, didDiscoverCharacteristicsForService service: CBService!, error: NSError!) {
    if serviceList[service.UUID] != nil && --characteristics == 0 {
      isPopulated = true
      serviceCallbacks.call(self.serviceList)
    }
  }
  
  func peripheral(peripheral: CBPeripheral!, didUpdateValueForCharacteristic characteristic: CBCharacteristic!, error: NSError!) {
    if error == nil {
      var value = Utilities.getValue(characteristic)
      if let callback = readCallbacks[characteristic.UUID] {
        readCallbacks.removeValueForKey(characteristic.UUID)
        callback.call(error == nil ? value : nil)
      } else {
        delegate?.onNotification(error == nil, uuid: characteristic.UUID, data: value)
      }
    } else {
      if let callback = readCallbacks[characteristic.UUID] {
        readCallbacks.removeValueForKey(characteristic.UUID)
        callback.call(nil)
      } else {
        delegate?.onNotification(true, uuid: characteristic.UUID, data: NSData())
      }
    }
  }
  
  func peripheral(peripheral: CBPeripheral!, didWriteValueForCharacteristic characteristic: CBCharacteristic!, error: NSError!) {
    delegate?.onWriteComplete(error == nil, uuid: characteristic.UUID)
  }
}
