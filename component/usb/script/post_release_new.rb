
# post_release.rb -r D:\test\rel6\7d62fSDK_2.0_LPCXpresso54608 -v 1.6.3 -b lpcxpresso54608

require "fileutils"

$BOARD = 'frdmk66f'
$root_dir = ""

def update_file(path, regular, replace)
    if File.exist?(path)
        File.open(path) do |fr|
            line = fr.read.gsub(regular, replace)
            File.open(path, "w") { |fw| fw.write(line) }
        end
        return true
    end
    return false
end

#update the USB stack version
def update_usb_stack_version()
    if ARGV.include?("-v")
        param = ARGV[ARGV.index("-v") + 1]
        if param.match(/[0-9]+\.[0-9]+\.[0-9]+/)
            regular = /([0-9]+)\.([0-9]+)\.([0-9]+)/
            regular.match(param)
            major = $1
            minor = $2
            bugfix = $3
            # puts "\r\nThe new version of USB stack is #{major}.#{minor}.#{bugfix}"
            # puts "#{$root_dir}/middleware/usb_#{param}/include/usb.h"
            if File.exist?("#{$root_dir}/middleware/usb/include/usb.h")
                update_file("#{$root_dir}/middleware/usb/include/usb.h",/USB_STACK_VERSION_MAJOR *\(.*\)/, "USB_STACK_VERSION_MAJOR \(#{major}U\)");
                update_file("#{$root_dir}/middleware/usb/include/usb.h",/USB_STACK_VERSION_MINOR *\(.*\)/, "USB_STACK_VERSION_MINOR \(#{minor}U\)");
                update_file("#{$root_dir}/middleware/usb/include/usb.h",/USB_STACK_VERSION_BUGFIX *\(.*\)/, "USB_STACK_VERSION_BUGFIX \(#{bugfix}U\)");
                update_file("#{$root_dir}/middleware/usb/include/usb.h",/USB_STACK_COMPONENT_VERSION MAKE_VERSION\(.*,.*,.*\)/, "USB_STACK_COMPONENT_VERSION MAKE_VERSION\(#{major}, #{minor}, #{bugfix}\)");
                # puts "The USB Stack version is updated!"
            else
                puts "Can't open the file #{$root_dir}/middleware/usb_#{param}/include/usb.h"
            end
            return
        end
    end
    puts("Invalid version parameter!")
end

def update_config_file_for_frdmk66f(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_frdmk66f("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmcimx6ul(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmcimx6ul("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE *\(.*\)/, "USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE *\(.*\)/, "USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evk_k32h844p(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evk_k32h844p("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(2U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmcimx6ull(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmcimx6ull("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE *\(.*\)/, "USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE *\(.*\)/, "USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1020(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1020("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1024(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1024("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1050(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1050("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1010(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1010("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1060(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1060("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1064(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1064("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkbimxrt1050(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkbimxrt1050("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(2U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt1015(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1015("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end
                           
def update_config_file_for_evkmimxrt1170(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt1170("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end

def update_config_file_for_frdmk28f(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_frdmk28f("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end
def update_config_file_for_frdmk28fa(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_frdmk28fa("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(1U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                        end
                        path = path.gsub("//", "/")
                        if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(1U\)");
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso54114(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54114("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso51u68(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso51u68("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso54608(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54608("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso55s69(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso55s69("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso55s28(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso55s28("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso5528(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso5528("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso55s16(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso55s16("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end
def update_config_file_for_lpcxpresso54618(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54618("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpcxpresso54s608(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54s608("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpcxpresso54628(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54628("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpcxpresso54018(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54018("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpcxpresso54s018(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54s018("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpc54018iotmodule(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpc54018iotmodule("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpc54s018iotmodule(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpc54s018iotmodule("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_lpcxpresso54s018m(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_lpcxpresso54s018m("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt685(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt685("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(1U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_host_hid_mouse")
                                    update_file("#{path}/#{sub}",/USB_HOST_CONFIG_BATTERY_CHARGER *\(.*\)/, "USB_HOST_CONFIG_BATTERY_CHARGER \(1U\)");
                                end
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_evkmimxrt595(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_evkmimxrt595("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if path.include?("usb_lpm_host_hid_mouse")
                    else
                        if path.include?("usb_keyboard2mouse")
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(1U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                            end
                        else
                            if "#{sub}" == "usb_device_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(0U\)");
                                update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(1U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_device_hid_mouse/bm") or path.include?("usb_device_hid_mouse\\bm")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if path.include?("usb_device_hid_mouse") or path.include?("usb_device_hid_mouse_lite")
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_CHARGER_DETECT *\(.*\)/, "USB_DEVICE_CONFIG_CHARGER_DETECT \(1U\)");
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                                if (path.include?("suspend"))
                                    # puts "Updating #{path}/#{sub}"
                                    update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                                end
                            elsif "#{sub}" == "usb_host_config.h"
                                # puts "Updating #{path}/#{sub}"
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(0U\)");
                                update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(1U\)");
                                path = path.gsub("//", "/")
                                if path.include?("usb_host_hid_mouse")
                                    update_file("#{path}/#{sub}",/USB_HOST_CONFIG_BATTERY_CHARGER *\(.*\)/, "USB_HOST_CONFIG_BATTERY_CHARGER \(1U\)");
                                end
                            end
                        end
                    end
                end
            end
        end
    end
end

def update_config_file_for_qn908xdk(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    #puts "[#{sub}]"
                    update_config_file_for_qn908xdk("#{path}/#{sub}")
                else
                    # puts "  |--#{sub}"
                    if "#{sub}" == "usb_device_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_KHCI *\(.*\)/, "USB_DEVICE_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_EHCI *\(.*\)/, "USB_DEVICE_CONFIG_EHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511FS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511FS \(1U\)");
                        update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_LPCIP3511HS *\(.*\)/, "USB_DEVICE_CONFIG_LPCIP3511HS \(0U\)");
                        if (path.include?("suspend"))
                            # puts "Updating #{path}/#{sub}"
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_SELF_POWER *\(.*\)/, "USB_DEVICE_CONFIG_SELF_POWER \(0U\)");
                            update_file("#{path}/#{sub}",/USB_DEVICE_CONFIG_DETACH_ENABLE *\(.*\)/, "USB_DEVICE_CONFIG_DETACH_ENABLE \(1U\)");
                        end
                    elsif "#{sub}" == "usb_host_config.h"
                        # puts "Updating #{path}/#{sub}"
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_KHCI *\(.*\)/, "USB_HOST_CONFIG_KHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_EHCI *\(.*\)/, "USB_HOST_CONFIG_EHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_OHCI *\(.*\)/, "USB_HOST_CONFIG_OHCI \(0U\)");
                        update_file("#{path}/#{sub}",/USB_HOST_CONFIG_IP3516HS *\(.*\)/, "USB_HOST_CONFIG_IP3516HS \(0U\)");
                    end
                end
            end
        end
    end
end

def update_for_board()
    if ARGV.include?("-b")
        param = ARGV[ARGV.index("-b") + 1]
        if param.match(/frdmk66f/)
            $BOARD = 'frdmk66f'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_frdmk66f("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmcimx6ul/)
            $BOARD = 'evkmcimx6ul'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmcimx6ul("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmcimx6ull/)
            $BOARD = 'evkmcimx6ull'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmcimx6ull("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evk_k32h844p/)
            $BOARD = 'evk_k32h844p'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evk_k32h844p("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1050/)
            $BOARD = 'evkmimxrt1050'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1050("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1060/)
            $BOARD = 'evkmimxrt1060'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1060("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1064/)
            $BOARD = 'evkmimxrt1064'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1064("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1020/)
            $BOARD = 'evkmimxrt1020'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1020("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1024/)
            $BOARD = 'evkmimxrt1024'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1024("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkbimxrt1050/)
            $BOARD = 'evkbimxrt1050'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkbimxrt1050("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1170/)
            $BOARD = 'evkmimxrt1170'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1170("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1010/)
            $BOARD = 'evkmimxrt1010'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1010("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt1015/)
            $BOARD = 'evkmimxrt1015'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt1015("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/frdmk28f/)
            $BOARD = 'frdmk28f'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_frdmk28f("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/frdmk28fa/)
            $BOARD = 'frdmk28fa'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_frdmk28fa("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54114/)
            $BOARD = 'lpcxpresso54114'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54114("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso51u68/)
            $BOARD = 'lpcxpresso51u68'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso51u68("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54608/)
            $BOARD = 'lpcxpresso54608'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54608("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso55s69/)
            $BOARD = 'lpcxpresso55s69'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso55s69("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso55s28/)
            $BOARD = 'lpcxpresso55s28'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso55s28("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso5528/)
            $BOARD = 'lpcxpresso5528'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso5528("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso55s16/)
            $BOARD = 'lpcxpresso55s16'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso55s16("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54618/)
            $BOARD = 'lpcxpresso54618'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54618("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54s608/)
            $BOARD = 'lpcxpresso54s608'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54s608("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54628/)
            $BOARD = 'lpcxpresso54628'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54628("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54018/)
            $BOARD = 'lpcxpresso54018'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54018("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54s018/)
            $BOARD = 'lpcxpresso54s018'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54s018("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpc54018iotmodule/)
            $BOARD = 'lpc54018iotmodule'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpc54018iotmodule("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpc54s018iotmodule/)
            $BOARD = 'lpc54s018iotmodule'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpc54s018iotmodule("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/lpcxpresso54s018m/)
            $BOARD = 'lpcxpresso54s018m'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_lpcxpresso54s018m("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt685/)
            $BOARD = 'evkmimxrt685'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt685("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/evkmimxrt595/)
            $BOARD = 'evkmimxrt595'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_evkmimxrt595("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        if param.match(/qn908x/)
            $BOARD = case param
            when /qn908xadk/
                'qn908xadk'
            when /qn908xbdk/
                'qn908xbdk'
            else
                'qn908xcdk'
            end
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                update_config_file_for_qn908xdk("#{$root_dir}/boards/#{$BOARD}/usb_examples")
                # puts "The config file for #{$BOARD} is updated!"
            else
                puts "Can't open the file #{$root_dir}/boards/#{$BOARD}/usb_examples"
            end
        end
        return
    end
    puts("Invalid board parameter!")
end

if ARGV.include?("-r")
    $root_dir = ARGV[ARGV.index("-r") + 1]
    # puts $root_dir
    if File.directory?("#{$root_dir}")
        if ARGV.include?("-b")
            update_for_board()
        elsif ARGV.include?("-v")
            update_usb_stack_version()
        end
    else
        puts("Can't open the file #{$root_dir}")
    end
else
    puts("Invalid root dir parameter!")
end

