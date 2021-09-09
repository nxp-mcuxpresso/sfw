
# post_release.rb -r D:\test\rel6\7d62fSDK_2.0_LPCXpresso54608 -b lpcxpresso54608

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

def update_pd_config_file_for_evkmimxrt1020_om13588(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_evkmimxrt1020_om13588("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}","\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_30\)", "\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_20\)");
                        update_file("#{path}/#{sub}",/PD_CONFIG_ENABLE_AUTO_POLICY *\(.*\)/, "PD_CONFIG_ENABLE_AUTO_POLICY \(0U\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_evkmimxrt1020_om13790host(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_evkmimxrt1020_om13790host("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}","\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_30\)", "\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_20\)");
                        update_file("#{path}/#{sub}",/PD_CONFIG_ENABLE_AUTO_POLICY *\(.*\)/, "PD_CONFIG_ENABLE_AUTO_POLICY \(0U\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_evkmimxrt1024_om13790host(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_evkmimxrt1024_om13790host("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}","\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_30\)", "\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_20\)");
                        update_file("#{path}/#{sub}",/PD_CONFIG_ENABLE_AUTO_POLICY *\(.*\)/, "PD_CONFIG_ENABLE_AUTO_POLICY \(0U\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_frdmkl27z_om13588(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_frdmkl27z_om13588("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}","\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_30\)", "\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_20\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_frdmkl27z_om13790host(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_frdmkl27z_om13790host("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}","\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_30\)", "\#define PD_CONFIG_REVISION \(PD_SPEC_REVISION_20\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_lpcxpresso54114_om13588(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_lpcxpresso54114_om13588("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}",/PD_CONFIG_PHY_LOW_POWER_LEVEL *\(.*\)/, "PD_CONFIG_PHY_LOW_POWER_LEVEL \(2\)");
                    end
                end
            end
        end
    end
end

def update_pd_config_file_for_lpcxpresso54114_om13790host(path)
    if File.directory?(path)
        Dir.entries(path).each do |sub|
            if sub != '.' && sub != '..'
                if File.directory?("#{path}/#{sub}")
                    update_pd_config_file_for_lpcxpresso54114_om13790host("#{path}/#{sub}")
                else
                    if "#{sub}" == "usb_pd_config.h"
                        update_file("#{path}/#{sub}",/PD_CONFIG_PHY_LOW_POWER_LEVEL *\(.*\)/, "PD_CONFIG_PHY_LOW_POWER_LEVEL \(2\)");
                    end
                end
            end
        end
    end
end

def update_for_board()
    if ARGV.include?("-b")
        param = ARGV[ARGV.index("-b") + 1]
        if param.match(/evkmimxrt1020/)
            $BOARD = 'evkmimxrt1020'
            $KIT = 'evkmimxrt1020_om13588'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
                update_pd_config_file_for_evkmimxrt1020_om13588("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd"
            end
            $KIT = 'evkmimxrt1020_om13790host'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
                update_pd_config_file_for_evkmimxrt1020_om13790host("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd"
            end
        end
        if param.match(/evkmimxrt1024/)
            $BOARD = 'evkmimxrt1024'
            $KIT = 'evkmimxrt1024_om13790host'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
                update_pd_config_file_for_evkmimxrt1024_om13790host("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd"
            end
        end
        if param.match(/frdmkl27z/)
            $BOARD = 'frdmkl27z'
            $KIT = 'frdmkl27z_om13588'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos")
                update_pd_config_file_for_frdmkl27z_om13588("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos"
            end
            $KIT = 'frdmkl27z_om13790host'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos")
                update_pd_config_file_for_frdmkl27z_om13790host("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd/freertos"
            end
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd_alt_mode_dp_host/freertos")
                update_pd_config_file_for_frdmkl27z_om13790host("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd_alt_mode_dp_host/freertos")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd_alt_mode_dp_host/freertos"
            end
        end
        if param.match(/lpcxpresso54114/)
            $BOARD = 'lpcxpresso54114'
            $KIT = 'lpcxpresso54114_om13588'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
                update_pd_config_file_for_lpcxpresso54114_om13588("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd"
            end
            $KIT = 'lpcxpresso54114_om13790host'
            if File.directory?("#{$root_dir}") and File.directory?("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
                update_pd_config_file_for_lpcxpresso54114_om13790host("#{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd")
            else
                puts "Can't open the file #{$root_dir}/boards/#{$KIT}/usb_examples/usb_pd"
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
        end
    else
        puts("Can't open the file #{$root_dir}")
    end
else
    puts("Invalid root dir parameter!")
end

