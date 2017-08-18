#
# Be sure to run `pod lib lint CursorBuffer.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'CursorBuffer'
  s.version          = '1.0'
  s.summary          = 'A FIFO buffer for Core Audio AudioBufferlists optimized for reading without comsuming.'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
A FIFO buffer for Core Audio AudioBufferlists optimized for reading without comsuming. Based on TPCircularBuffer.
                       DESC

  s.homepage         = 'https://github.com/dave234/CursorBuffer'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'dave234' => 'daveoneill234@gmail.com' }
  s.source           = { :git => 'https://github.com/dave234/CursorBuffer.git', :tag => s.version.to_s }

  s.ios.deployment_target = '4.3'
  s.osx.deployment_target = '10.8'

  s.source_files = '*.{c,h}'
  s.dependency 'TPCircularBuffer'
end
