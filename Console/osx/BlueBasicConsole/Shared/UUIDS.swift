//
//  Uuids.swift
//  BlueBasicConsole
//
//  Created by tim on 9/25/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

struct UUIDS {
  
  static let commsServiceUUID = CBUUID(string: "25FB9E91-1616-448D-B5A3-F70A64BDA73A")
  static let inputCharacteristicUUID = CBUUID(string: "C3FBC9E2-676B-9FB5-3749-2F471DCF07B2")
  static let outputCharacteristicUUID = CBUUID(string: "D6AF9B3C-FE92-1CB2-F74B-7AFB7DE57E6D")
  
  static let oadServiceUUID = CBUUID(string: "F000FFC0-0451-4000-B000-000000000000")
  static let imgIdentityUUID = CBUUID(string: "F000FFC1-0451-4000-B000-000000000000")
  static let imgBlockUUID = CBUUID(string: "F000FFC2-0451-4000-B000-000000000000")
  
  static let deviceInfoServiceUUID = CBUUID(string: "180A")
  static let firmwareRevisionUUID = CBUUID(string: "2A26")
  
}