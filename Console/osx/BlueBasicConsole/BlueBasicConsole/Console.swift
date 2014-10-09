//
//  Console.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import Cocoa
import CoreBluetooth

class Console: NSObject, NSTextViewDelegate, DeviceDelegate, ConsoleProtocol {
  
  let statusField: NSTextField
  let console: NSTextView
  var current: Device?
  
  var inputCharacteristic: CBCharacteristic?
  var outputCharacteristic: CBCharacteristic?
  var pending = ""
  var recoveryMode = false
  
  var delegate: ConsoleDelegate?

  init(console: NSTextView, status: NSTextField) {
    self.statusField = status
    self.console = console
    self.status = "Not connected"
    super.init()
    console.automaticQuoteSubstitutionEnabled = false
    console.font = NSFont(name: "Menlo Regular", size: 11)
    console.delegate = self
  }

  var status: String {
    didSet {
      statusField.stringValue = status
      if isConnected && !isRecoveryMode {
        console.editable = true
        console.window?.makeFirstResponder(console)
      } else {
        console.editable = false
      }
    }
  }
  
  // Workaround
  func setStatus(status: String) {
    self.status = status
  }
  
  // Workaround
  func setDelegate(delegate: ConsoleDelegate) {
    self.delegate = delegate
  }
  
  var isConnected: Bool {
    get {
      return status == "Connected" || status == "Upgrade available"
    }
  }
  
  var isRecoveryMode: Bool {
    get {
      return recoveryMode
    }
  }
  
  func connectTo(device: Device, onConnected: CompletionHandler? = nil) {
    disconnect() {
      success in
      self.status = "Connecting..."
      self.current = device
      device.connect() {
        success in
        if success {
          device.services() {
            list in
            if list[UUIDS.commsServiceUUID] != nil {
              self.inputCharacteristic = list[UUIDS.commsServiceUUID]!.characteristics[UUIDS.inputCharacteristicUUID]
              self.outputCharacteristic = list[UUIDS.commsServiceUUID]!.characteristics[UUIDS.outputCharacteristicUUID]
              self.current!.read(self.inputCharacteristic!) {
                data in
                if data == nil {
                  if list[UUIDS.oadServiceUUID] != nil {
                    self.current!.delegate = self
                    self.recoveryMode = true
                    self.status = "Recovery mode"
                    onConnected?(true)
                  } else {
                    self.status = "Failed"
                    self.disconnect()
                    onConnected?(false)
                  }
                } else {
                  self.status = "Connected"
                  self.current!.delegate = self
                  self.current!.notify(UUIDS.inputCharacteristicUUID, serviceUUID: UUIDS.commsServiceUUID)
                  onConnected?(true)
                }
              }
            } else if list[UUIDS.oadServiceUUID] != nil {
              self.current!.delegate = self
              self.recoveryMode = true
              self.status = "Recovery mode"
              onConnected?(true)
            } else {
              self.status = "Unsupported"
              self.disconnect()
              onConnected?(false)
            }
          }
        } else {
          self.status = "Failed"
          onConnected?(false)
        }
      }
    }
  }
  
  func onWriteComplete(success: Bool, uuid: CBUUID) {
    delegate?.onWriteComplete(uuid)
  }
  
  func onNotification(success: Bool, uuid: CBUUID, data: NSData) {
    switch uuid {
    case UUIDS.inputCharacteristicUUID:
      if delegate == nil || delegate!.onNotification(uuid, data: data) {
        var str = NSString(data: data, encoding: NSASCIIStringEncoding)!
        console.replaceCharactersInRange(NSMakeRange(console.string!.utf16Count, 0), withString: str)
        console.scrollRangeToVisible(NSMakeRange(console.string!.utf16Count, 0))
        console.needsDisplay = true
      }
    default:
      break
    }
  }
  
  func onDisconnect() {
    if let old = current {
      old.connect()
    }
  }

  func disconnect(_ onDisconnect: CompletionHandler? = nil) {
    if let old = current {
      current = nil
      delegate = nil
      status = "Not connected"
      recoveryMode = false
      old.delegate = nil
      old.disconnect(onDisconnect)
    } else {
      onDisconnect?(true)
    }
  }
  
  func write(_ str: String = "\n") {
    for ch in str {
      pending.append(ch)
      if ch == "\n" || pending.utf16Count > 64 {
        current!.write(pending.dataUsingEncoding(NSASCIIStringEncoding, allowLossyConversion: false)!, characteristic: outputCharacteristic!, type: .WithResponse)
        pending = ""
      }
    }
  }
  
  func textView(textView: NSTextView!, shouldChangeTextInRange affectedCharRange: NSRange, replacementString: String!) -> Bool {
    var consoleCount = console.string!.utf16Count
    if current == nil {
      return false
    } else if replacementString.utf16Count > 0 {
      write(replacementString)
      if affectedCharRange.location == consoleCount {
        return true
      } else {
        textView.replaceCharactersInRange(NSMakeRange(consoleCount, 0), withString: replacementString)
        textView.setSelectedRange(NSMakeRange(console.string!.utf16Count, 0))
        return false
      }
    } else if affectedCharRange.location == consoleCount - 1 && pending.utf16Count > 0 {
      pending.removeAtIndex(pending.endIndex.predecessor())
      return true
    } else {
      return false
    }
  }

}


