# frozen_string_literal: true

require_relative 'explorerlens/version'

module ExplorerLens
  def self.run(args)
    require 'rbconfig'

    unless RbConfig::CONFIG['host_os'].match?(/mswin|mingw|cygwin/)
      warn 'ExplorerLens lens.exe is a Windows-only binary.'
      warn 'Download it from: https://github.com/RajwanYair/ExplorerLens.io/releases'
      exit 1
    end

    lens_exe = File.join(__dir__, '..', '..', 'vendor', 'lens.exe')

    unless File.exist?(lens_exe)
      warn "lens.exe not found at #{lens_exe}"
      warn "Install via: gem install explorerlens  (downloads binary automatically)"
      exit 1
    end

    exec lens_exe, *args
  end
end
