--[[
EnvUtils2: Environmental Variable Utilities
Written by AJ Kelly of KKI Labs / Version 2.0

This code is a rewrite of what typically exists in EnvUtils.lua
(as seen in dubaiOne), hereafter referred to as EnvUtils1.

I felt it was time for a simplification of the code.
This new version should also work better and be less confusing.
--]]

-- Env table global
envTable = GAMESTATE:Env();

-- setenv(name,value)
-- Sets aside an entry for /name/ and puts /value/ into it.
-- Unlike EnvUtils1, this is the only setenv function available to you.
-- If you need to store more than one value, you're welcome to use a
-- table as /value/, it should work just fine.
function setenv(name,value)
	envTable[name] = value;
end;

-- getenv(name)
-- This will return whatever value is at envTable[name].
function getenv(name)
	return envTable[name];
end;

--[[
Copyright © 2008 AJ Kelly/KKI Labs
Use freely.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]