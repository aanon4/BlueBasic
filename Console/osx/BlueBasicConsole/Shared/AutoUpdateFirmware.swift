//
//  AutoUpdateFirmware.swift
//  BlueBasicConsole
//
//  Created by tim on 10/8/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

var _firmwareVersion: String?
var _firmwareBlob: NSData?

class AutoUpdateFirmware {
  
  let baseURL = "https://github.com/aanon4/BlueBasic/raw/master/hex/BlueBasic-"

  let console: ConsoleProtocol
  let device: Device
  
  init(console: ConsoleProtocol) {
    self.console = console
    self.device = console.current!
  }
  
  func detectUpgrade(onComplete: CompletionHandler) {
    device.services() {
      list in
      let revision = list[UUIDS.deviceInfoServiceUUID]!.characteristics[UUIDS.firmwareRevisionUUID]!
      self.device.read(revision) {
        data in
        if data == nil {
          onComplete(false)
        } else {
          self.fetchLatestVersion(NSString(data: data!, encoding: NSASCIIStringEncoding)!) {
            success in
            dispatch_async(dispatch_get_main_queue()) {
              onComplete(success)
            }
          }
        }
      }
    }
  }
  
  func upgrade(onComplete: CompletionHandler? = nil) {
    if _firmwareBlob == nil {
      onComplete?(false)
    } else {
      Firmware(console).upgrade(_firmwareBlob!, onComplete: onComplete)
    }
  }
  
  func fetchLatestVersion(currentVersion: String, onComplete: CompletionHandler) {
    if _firmwareVersion == currentVersion {
      onComplete(false)
    } else if _firmwareVersion != nil {
      onComplete(_firmwareBlob != nil)
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
            _firmwareVersion = currentVersion
            onComplete(false)
          } else {
            (session.dataTaskWithURL(NSURL(string: self.baseURL + versionParts[0] + ".bin")!) {
              data, response, error in
              if error == nil && data != nil {
                _firmwareBlob = data
                _firmwareVersion = "\(versionParts[0])/\(versionInfo)"
              }
              onComplete(error == nil)
            }).resume()
          }
        }
      }).resume()
    }
  }
}

