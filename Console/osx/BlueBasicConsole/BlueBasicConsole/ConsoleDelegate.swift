//
//  ConsoleDelegate.swift
//  BlueBasicConsole
//
//  Created by tim on 9/24/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

protocol ConsoleDelegate {
  
  func onNotification(uuid: CBUUID, data: NSData) -> Bool
  func onWriteComplete(uuid: CBUUID)
  
}