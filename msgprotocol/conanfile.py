from conans import ConanFile, Meson

class exaocbot_msgprotocol(ConanFile):
	name = "exaocbot_msgprotocol"
	version = "0.1.0"
	requires = []
	generators = ["pkg_config"]
	exports_sources = "src/*"

	def build(self):
		meson = Meson(self)

	def package(self):
		self.copy("*.h", dst="include", src="src")
