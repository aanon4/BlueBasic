//
//  DeviceService.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class DeviceService {
  
  var device: Device
  var service: CBService
  
  init(device: Device, service: CBService) {
    self.device = device
    self.service = service
  }
  
  var characteristics: [CBUUID: CBCharacteristic] {
    get {
      var characteristics = [CBUUID: CBCharacteristic]()
      for characteristic in service.characteristics as [CBCharacteristic] {
        characteristics[characteristic.UUID] = characteristic
      }
      return characteristics
    }
  }
}