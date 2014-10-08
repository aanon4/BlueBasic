//
//  AutoUpdateFirmware.swift
//  BlueBasicConsole
//
//  Created by tim on 10/8/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class AutoUpdateFirmware {
  
  let baseURL = "https://github.com/aanon4/BlueBasic/raw/master/hex/BlueBasic-"

  let console: Console
  let device: Device
  var version: String?
  var firmware: NSData?
  
  init(console: Console) {
    self.console = console
    self.device = console.current!
  }
  
  func detectUpgrade(onComplete: CompletionHandler) {
    if console.isUpgradable {
      // We cannot autodetect upgrade if the board is already in upgrade mode :-(
      onComplete(false)
    } else {
      device.services() {
        list in
        let revision = list[UUIDS.deviceInfoServiceUUID]!.characteristics[UUIDS.firmwareRevisionUUID]!
        self.device.read(revision) {
          data in
          if data == nil {
            onComplete(false)
          } else {
            self.fetchLatestVersion(NSString(data: data!, encoding: NSASCIIStringEncoding)!, onComplete)
          }
        }
      }
    }
  }
  
  func upgrade(onComplete: CompletionHandler? = nil) {
    if firmware == nil {
      onComplete?(false)
    } else {
      Firmware(console).upgrade(firmware!, onComplete: onComplete)
    }
  }
  
  func fetchLatestVersion(currentVersion: String, onComplete: CompletionHandler) {
    if version == currentVersion {
      onComplete(false)
    } else if version != nil {
      onComplete(firmware != nil)
    } else {
      var session = NSURLSession(configuration: NSURLSessionConfiguration.defaultSessionConfiguration())

      var versionParts = currentVersion.componentsSeparatedByString("/")
      (session.dataTaskWithURL(NSURL(string: baseURL + versionParts[0] + ".version")!) {
        data, response, error in
        if error != nil || data == nil {
          onComplete(false)
        } else {
          var versionInfo = NSString(data: data!, encoding: NSASCIIStringEncoding)!.substringToIndex(14)
          if versionInfo <= versionParts[1] {
            self.version = currentVersion
            onComplete(false)
          } else {
            (session.dataTaskWithURL(NSURL(string: self.baseURL + versionParts[0] + ".bin")!) {
              data, response, error in
              if error == nil && data != nil {
                self.firmware = data
                self.version = "\(versionParts[0])/\(versionInfo)"
              }
              onComplete(error == nil)
            }).resume()
          }
        }
      }).resume()
    }
  }
}

