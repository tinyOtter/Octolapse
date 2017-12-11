/// Create our stabilizations view model
$(function() {
    Octolapse.CameraProfileViewModel = function(values) {
        self = this
        self.name = ko.observable(values.name);
        self.guid = ko.observable(values.guid);
        self.apply_settings_before_print = ko.observable(values.apply_settings_before_print);
        self.address = ko.observable(values.address);
        self.snapshot_request_template = ko.observable(values.snapshot_request_template);
        self.ignore_ssl_error = ko.observable(values.ignore_ssl_error);
        self.username = ko.observable(values.username);
        self.password = ko.observable(values.password);
        self.brightness = ko.observable(values.brightness);
        self.brightness_request_template = ko.observable(values.brightness_request_template);
        self.contrast = ko.observable(values.contrast);
        self.contrast_request_template = ko.observable(values.contrast_request_template);
        self.saturation = ko.observable(values.saturation);
        self.saturation_request_template = ko.observable(values.saturation_request_template);
        self.white_balance_auto = ko.observable(values.white_balance_auto);
        self.white_balance_auto_request_template = ko.observable(values.white_balance_auto_request_template);
        self.gain = ko.observable(values.gain);
        self.gain_request_template = ko.observable(values.gain_request_template);
        self.powerline_frequency = ko.observable(values.powerline_frequency);
        self.powerline_frequency_request_template = ko.observable(values.powerline_frequency_request_template);
        self.white_balance_temperature = ko.observable(values.white_balance_temperature);
        self.white_balance_temperature_request_template = ko.observable(values.white_balance_temperature_request_template);
        self.sharpness = ko.observable(values.sharpness);
        self.sharpness_request_template = ko.observable(values.sharpness_request_template);
        self.backlight_compensation_enabled = ko.observable(values.backlight_compensation_enabled);
        self.backlight_compensation_enabled_request_template = ko.observable(values.backlight_compensation_enabled_request_template);
        self.exposure_type = ko.observable(values.exposure_type);
        self.exposure_type_request_template = ko.observable(values.exposure_type_request_template);
        self.exposure = ko.observable(values.exposure);
        self.exposure_request_template = ko.observable(values.exposure_request_template);
        self.exposure_auto_priority_enabled = ko.observable(values.exposure_auto_priority_enabled);
        self.exposure_auto_priority_enabled_request_template = ko.observable(values.exposure_auto_priority_enabled_request_template);
        self.pan = ko.observable(values.pan);
        self.pan_request_template = ko.observable(values.pan_request_template);
        self.tilt = ko.observable(values.tilt);
        self.tilt_request_template = ko.observable(values.tilt_request_template);
        self.autofocus_enabled = ko.observable(values.autofocus_enabled);
        self.autofocus_enabled_request_template = ko.observable(values.autofocus_enabled_request_template);
        self.focus = ko.observable(values.focus);
        self.focus_request_template = ko.observable(values.focus_request_template);
        self.zoom = ko.observable(values.zoom);
        self.zoom_request_template = ko.observable(values.zoom_request_template);
        self.led1_mode = ko.observable(values.led1_mode);
        self.led1_mode_request_template = ko.observable(values.led1_mode_request_template);
        self.led1_frequency = ko.observable(values.led1_frequency);
        self.led1_frequency_request_template = ko.observable(values.led1_frequency_request_template);
        self.jpeg_quality = ko.observable(values.jpeg_quality);
        self.jpeg_quality_request_template = ko.observable(values.jpeg_quality_request_template);
    }
    Octolapse.CameraProfileValidationRules = {
        rules: {
            name: "required"
        },
        messages: {
            name: "Please enter a name for your profile",
        }
    };
});

