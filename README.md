# SyntaWiFi
Control your telescope wirelessly with ESP8266.

I use this sektch on Wemos D1 Mini to controll my AstroEQ mount wirelessly with SynScan Pro app. It's based on existing project but adapted to wrok on Wemos D1 Mini (wemos can not work with swapped pins and TTL shifter - this will burn the board, therefore in this prject I use standard UART, and logger had to be removed completley)

Connections:

Wemos GND <> Mount GND
Wemos RX <TTL Shifter> Mount TX
Wemos TX <TTL Shifter> Mount RX

Wemos D1 Mini UART works on 3.3V logic level therefore bi-directional logic level shifter is required in between (3.3V <-> 5V). TTL shifter can be powered from Wemos D1 Mini PCB (from pins 5V, 3.3V and GND), but Wemos itself may consume lot of current (up to 660 mA) and therefore it must be powered from standalone 5V power supply (NOT FROM ARDUINO 5V PIN!)

WiFi SSID: SynScan_WiFi_1234
WiFi password: password

Device IP is 192.168.4.1 and it's listening on port 11880/UDP acting as transparent serial bridge. You can even connect it as VCOM with USR-VCOM or HW Virtual Serial Port or similar software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
