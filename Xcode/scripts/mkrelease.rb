# 'fileutils' is used to recursively copy directories
# 'tmpdir' is used to create and work with temporary directories
require 'fileutils'
require 'tmpdir'

# cd to the StepMania 5 src directory
Dir.chdir "../../src"

# initialize empty strings
family, version = ""

# open ProductInfo.h in read-only mode
File.open("#{Dir.pwd}/ProductInfo.h", "r") do |f|
	# read each line, matching for product family and version
	f.each do |line|
		if line.match( /^#define\s+PRODUCT_FAMILY_BARE\s+(.*?)\s*$/ )
			family = $1;

		elsif line.match( /^#define\s+PRODUCT_VER_BARE\s+(.*?)\s*$/ )
			version = $1;
		end
	end	
end

# replace whitespace with hyphens in version string
version.gsub!(/\s+/,"-")

# the name of the .dmg file
name = "#{family}-#{version}-mac.dmg"

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
	FileUtils.mkdir_p("#{temp}/#{family}-#{version}/#{family}-#{version}/")
	
	# loop through the directories array
	directories.each do |directory|
		# recursively copy each directory into our temp directory
		FileUtils.cp_r directory, "#{temp}/#{family}-#{version}/#{family}-#{version}/", :verbose => true
	end

	#construct the shell command that will create the dmg
	cmd = "hdiutil create #{Dir.pwd}/#{name} -srcfolder #{temp}/#{family}-#{version} -ov"

	#execute the command in a subshell
	system( cmd )
}