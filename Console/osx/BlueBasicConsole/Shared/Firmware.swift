//
//  Firmware.swift
//  BlueBasicConsole
//
//  Created by tim on 9/24/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class Firmware: ConsoleDelegate {

  let console: ConsoleProtocol
  var complete: CompletionHandler? = nil
  let device: Device
  var firmware : NSData?
  var wrote = 0
  var written = 0
  var blockCharacteristic: CBCharacteristic?
  
  init(_ console: ConsoleProtocol) {
    self.console = console
    self.device = console.current!
  }
  
  func upgrade(firmware: NSData, onComplete: CompletionHandler? = nil) {
    
    self.firmware = firmware
    self.complete = onComplete
    
    if console.isRecoveryMode {
      flash()
    } else {
      console.setStatus("Rebooting")
      console.write("REBOOT UP\n")
      // Wait a moment to give device chance to reboot
      dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1_000_000_000), dispatch_get_main_queue()) {
        self.console.disconnect() {
          success in
          self.console.connectTo(self.device) {
            success in
            self.flash()
          }
        }
      }
    }
  }
  
  func onWriteComplete(uuid: CBUUID) {
    
    let blocksize = 16
    let countsize = 16

    switch uuid {
    case UUIDS.imgIdentityUUID:
      var nrblocks = (firmware!.length + blocksize - 1) / blocksize
      for i in 0...nrblocks-1 {
        var block = NSMutableData(capacity: blocksize + 2)!
        var blockheader = [ Byte(i & 255), Byte(i >> 8) ]
        block.appendBytes(blockheader, length: 2)
        block.appendData(self.firmware!.subdataWithRange(NSMakeRange(i * blocksize, blocksize)))
        if i < nrblocks - 1 && i % countsize != 0 {
          device.write(block, characteristic: blockCharacteristic!, type: .WithoutResponse)
        } else {
          device.write(block, characteristic: blockCharacteristic!, type: .WithResponse)
          wrote++
        }
      }
    case UUIDS.imgBlockUUID:
      written++
      if written == wrote - 1 { // Last ack is always lost as device reboots
        console.setStatus("Waiting...")
        // Wait for 5 seconds to give last writes change to finish
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5_000_000_000), dispatch_get_main_queue()) {
          self.console.disconnect() {
            success in
            self.console.connectTo(self.device, self.complete)
          }
        }
      } else {
        console.setStatus(String(format: "Upgrading...%d%%", 100 * written / wrote))
      }
      break
    default:
      break
    }
  }

  func onNotification(uuid: CBUUID, data: NSData) -> Bool {
    return false
  }
  
  func flash() {

    device.services() {
      list in
      
      self.console.setDelegate(self)

      self.blockCharacteristic = list[UUIDS.oadServiceUUID]!.characteristics[UUIDS.imgBlockUUID]!
      
      var identCharacteristic = list[UUIDS.oadServiceUUID]!.characteristics[UUIDS.imgIdentityUUID]!
      let header = self.firmware!.subdataWithRange(NSMakeRange(4, 8)) // version:2, length:2, uid:4
      self.device.write(header, characteristic: identCharacteristic, type: .WithResponse)
    }
  }
  
}
