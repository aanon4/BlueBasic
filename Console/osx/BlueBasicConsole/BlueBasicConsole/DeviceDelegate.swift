//
//  DeviceDelegate.swift
//  BlueBasicConsole
//
//  Created by tim on 9/25/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

protocol DeviceDelegate {
  
  func onDisconnect()
  func onNotification(success: Bool, uuid: CBUUID, data: NSData)
  func onWriteComplete(success: Bool, uuid: CBUUID)
  
}