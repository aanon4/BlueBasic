//
//  DetailViewController.swift
//  BlueBasicIOS
//
//  Created by tim on 10/3/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import UIKit
import CoreBluetooth

var _current: Device?
var _currentOwner: UIViewController?

class DetailViewController: UIViewController, UITextViewDelegate, DeviceDelegate, ConsoleProtocol {

  @IBOutlet weak var console: UITextView!

  var inputCharacteristic: CBCharacteristic?
  var outputCharacteristic: CBCharacteristic?
  var pending = ""
  
  var keyboardOpen: CGRect? = nil
  
  var delegate: ConsoleDelegate?
  
  var autoUpgrade: AutoUpdateFirmware?
  var recoveryMode = false
  
  var detailItem: AnyObject? {
    didSet {
      connectTo(detailItem as Device) {
        success in
        if success {
          self.autoUpgrade = AutoUpdateFirmware(console: self)
          self.autoUpgrade!.detectUpgrade() {
            needupgrade in
            if needupgrade {
              self.status = "Upgrade available"
              self.navigationItem.rightBarButtonItem = UIBarButtonItem(title: "Upgrade", style: .Plain, target: self, action: "upgrade")
            } else {
              self.autoUpgrade = nil
              self.navigationItem.rightBarButtonItem = nil
            }
          }

        }
      }
    }
  }
  
  var current: Device? {
    get {
      return _current
    }
    set {
      _currentOwner = self
      _current = newValue
    }
  }

  override func viewDidLoad() {
    super.viewDidLoad()
    console.dataDetectorTypes = .None
    console.delegate = self
    console.layoutManager.allowsNonContiguousLayout = false // Fix scroll jump when keyboard dismissed
    self.navigationItem.title = status
  }
  
  override func viewWillAppear(animated: Bool) {
    super.viewWillAppear(animated)
    NSNotificationCenter.defaultCenter().addObserver(self, selector: "keyboardDidShow:", name: "UIKeyboardDidShowNotification", object: nil)
    NSNotificationCenter.defaultCenter().addObserver(self, selector: "keyboardDidHide:", name: "UIKeyboardDidHideNotification", object: nil)
  }
  
  override func viewDidAppear(animated: Bool) {
    super.viewDidAppear(animated)
    if detailItem == nil {
      popover?.presentPopoverFromBarButtonItem((view.window!.rootViewController as UISplitViewController).displayModeButtonItem(), permittedArrowDirections: .Any, animated: true)
    }
  }

  override func viewWillDisappear(animated: Bool) {
    super.viewWillDisappear(animated)
    NSNotificationCenter.defaultCenter().removeObserver(self, name: "UIKeyboardDidShowNotification", object: nil)
    NSNotificationCenter.defaultCenter().removeObserver(self, name: "UIKeyboardDidHideNotification", object: nil)
    resignActive()
  }

  override func didReceiveMemoryWarning() {
    super.didReceiveMemoryWarning()
  }

  // MARK: - Console mechanics
  
  var status: String = "Not connected" {
    didSet {
      self.navigationItem.title = status
      if isConnected && !isRecoveryMode {
        console?.editable = true
        console.becomeFirstResponder()
      } else {
        console?.editable = false
      }
    }
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
  
  // Workaround
  func setStatus(status: String) {
    self.status = status
  }
  
  // Workaround
  func setDelegate(delegate: ConsoleDelegate) {
    self.delegate = delegate
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
              if self.current == nil {
                self.status = "Failed"
                onConnected?(false)
              } else {
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
        console.selectedRange = NSMakeRange(console.text!.utf16Count, 0)
        console.insertText(str)
        console.scrollRangeToVisible(NSMakeRange(console.text!.utf16Count, 0))
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
      old.disconnect() {
        success in
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1_000_000_000), dispatch_get_main_queue()) {
          if onDisconnect != nil {
            onDisconnect!(success)
          }
        }
      }
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
  
  func textView(textView: UITextView, shouldChangeTextInRange range: NSRange, replacementText text: String) -> Bool {
    if current == nil {
      return false
    } else if text.utf16Count > 0 {
      write(text)
      if range.location == console.text.utf16Count {
        return true
      } else {
        console.selectedRange = NSMakeRange(console.text!.utf16Count, 0)
        console.insertText(text)
        console.scrollRangeToVisible(NSMakeRange(console.text.utf16Count, 0))
        return false
      }
    } else if range.location == console.text.utf16Count - 1 && pending.utf16Count > 0 {
      pending.removeAtIndex(pending.endIndex.predecessor())
      return true
    } else {
      return false
    }
  }
  
  func resignActive() {
    if _currentOwner == self {
      disconnect()
    }
  }

  func upgrade() {
    if autoUpgrade != nil {
      let alert = UIAlertController(title: "Upgrade?", message: "Do you want to upgrade the device firmware?", preferredStyle: .Alert)
      alert.addAction(UIAlertAction(title: "OK", style: .Destructive, handler: {
        action in
        self.navigationItem.rightBarButtonItem = nil
        UIApplication.sharedApplication().idleTimerDisabled = true
        self.autoUpgrade!.upgrade() {
          success in
          UIApplication.sharedApplication().idleTimerDisabled = false
        }
      }))
      alert.addAction(UIAlertAction(title: "Cancel", style: .Default, handler: {
        action in
      }))
      self.presentViewController(alert, animated: true, completion: nil)
    }
  }

  
  func keyboardDidShow(notification: NSNotification) {
    if keyboardOpen == nil {
      let info = notification.userInfo?[UIKeyboardFrameEndUserInfoKey] as NSValue
      let size = info.CGRectValue().size

      var frame = console.frame
      keyboardOpen = frame
      let bottom = UIScreen.mainScreen().bounds.size.height - (frame.origin.y + frame.size.height)
      frame.size.height -= (size.height - bottom) + console.font.lineHeight
      console.frame = frame

      console.selectedRange = NSMakeRange(console.text!.utf16Count, 0)
      console.scrollRangeToVisible(NSMakeRange(console.text.utf16Count, 0))
    }
  }
  
  func keyboardDidHide(notification: NSNotification) {
    if keyboardOpen != nil {
      self.console.frame.size.height = self.keyboardOpen!.size.height
      self.keyboardOpen = nil
      self.console.scrollRangeToVisible(NSMakeRange(self.console.text.utf16Count, 0))
    }
  }

}
