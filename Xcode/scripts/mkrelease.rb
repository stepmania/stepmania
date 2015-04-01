# 'fileutils' is used to recursively copy directories
# 'tmpdir' is used to create and work with temporary directories
require 'fileutils'
require 'tmpdir'

# cd to the StepMania 5 src directory
Dir.chdir "../../src"

# check the command line for the presence of the "nightly" option, used
# if we are bundling a nightly build as opposed to an official release
nightly = (true and ARGV[0] == "nightly") or false

# initialize empty strings
family, version, date = ""

# if this is to be a nightly build, store the system date;
# we'll use it below to name the bundle
if nightly
	time = time = Time.new
	date = "#{time.day}-#{time.month}-#{time.year}"
end

# open ProductInfo.h in read-only mode
File.open("#{Dir.pwd}/ProductInfo.h", "r") do |f|
	# read each line, matching for product family
	f.each do |line|
		if line.match( /^#define\s+PRODUCT_FAMILY_BARE\s+(.*?)\s*$/ )
			family = $1
		end
	end
end

# Determine if the Cmake generated verstub.cpp is available.
File.open("#{Dir.pwd}/verstub.cpp", "r") do |verFile|
  verFile.each do |verLine|
    if verLine.match( /^extern char const \* const product_version \= "(.*?)"/ )
      version = $1
    end
  end
end

if (version.length == 0)
  # open ver.h in read-only mode
  File.open("#{Dir.pwd}/ver.h", "r") do |f|
    # read each line, matching for product version
    f.each do |line|
      if line.match( /^#define\s+product_version\s+"(.*?)"/ )
        version = $1
      end
    end
  end
end

# replace whitespace with hyphens in the version string
version.gsub!(/\s+/,"-")

# the name of what we are bundling
if nightly
	version = version.partition("-")[0]
	name = "#{family}-#{version}-#{date}"
else
	name = "#{family}-#{version}"
end

# a list of directories we want to include in our .dmg
directories = [ "Announcers", "BackgroundEffects", "BackgroundTransitions",
		"BGAnimations", "Characters", "Courses", "Data", "Docs", "Manual",
		"NoteSkins", "Scripts", "Songs", "StepMania.app", "Themes" ]

# cd back to the root of the StepMania 5 directory
Dir.chdir ".."

# create a temp directory; this will be automatically deleted when the block completes
Dir.mktmpdir {|temp|

	# nest two directories named by family and version within the temp directory
	# the outer will become the root of the dmg
	# the inner will neatly tidy all the contents together so users can easily drag/drop everything at once
	FileUtils.mkdir_p("#{temp}/#{name}/#{name}/")

	# loop through the directories array
	directories.each do |directory|
		# recursively copy each directory into our temp directory
		FileUtils.cp_r directory, "#{temp}/#{name}/#{name}/", :verbose => true
	end

	#construct the shell command that will create the dmg
	cmd = "hdiutil create #{Dir.pwd}/#{name}-mac.dmg -srcfolder #{temp}/#{name} -ov"

	#execute the command in a subshell
	system( cmd )
}