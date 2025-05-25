import re
import os
from send2trash import send2trash

def customize_xmake_template(template_path, output_path):
    # Read template file
    with open(template_path, 'r') as f:
        template = f.read()

    # Gather user inputs
    project_name = input("Project name: ").strip()
    target_name = input(f"Target name (leave blank to use '{project_name}'): ").strip() or project_name
    version = input("Version string (e.g. 1.0.0): ").strip()

    # Config options
    need_ini = input("Need INI config? (y/n): ").lower() == 'y'
    need_json = input("Need JSON config? (y/n): ").lower() == 'y'
    need_toml = input("Need TOML config? (y/n): ").lower() == 'y'

    # Apply substitutions
    template = re.sub(r'set_project\(".*?"\)', f'set_project("{project_name}")', template)
    template = re.sub(r'target\(".*?"\)', f'target("{target_name}")', template)
    template = re.sub(r'name\s*=\s*".*?"', f'name = "{target_name}"', template)
    template = re.sub(r'set_version\(".*?"\)', f'set_version("{version}")', template)
    template = re.sub(r'set_config\("rex_.*?", true\)\n', '', template)

    # Build config lines
    config_lines = ""
    if need_ini:
        config_lines += 'set_config("rex_ini", true)\n'
    if need_json:
        config_lines += 'set_config("rex_json", true)\n'
    if need_toml:
        config_lines += 'set_config("rex_toml", true)\n'
    
    # add extra files to the template
    extra_lines = ""
    if need_ini:
        extra_lines += 'add_extrafiles("release/**.ini")\n'
    if need_json:
        extra_lines += 'add_extrafiles("release/**.json")\n'
    if need_toml:
        extra_lines += 'add_extrafiles("release/**.toml")\n'
        
    # Insert config and extra file lines before -- targets
    template = re.sub(r'(-- targets)', config_lines + extra_lines + r'\1', template)

    # Write result to output file
    with open(output_path, 'w') as f:
        f.write(template)

    print(f"\n✅ New xmake.lua generated at: {output_path}")
    
    # Create src/release directory if it doesn't exist
    release_dir = "release"
    os.makedirs(release_dir, exist_ok=True)

    # Create config files based on enabled options
    if need_ini:
        with open(os.path.join(release_dir, f"{target_name}.ini"), 'w') as f:
            f.write("; Example INI config\n[General]\nSetting=Value\n")
    if need_json:
        with open(os.path.join(release_dir, f"{target_name}.json"), 'w') as f:
            f.write('{\n    "setting": "value"\n}\n')
    if need_toml:
        with open(os.path.join(release_dir, f"{target_name}.toml"), 'w') as f:
            f.write('# Example TOML config\n[general]\nsetting = "value"\n')

    print(f"✅ Config files created in: {release_dir}")
    

if __name__ == "__main__":
    template_file = "xmake_template.lua"
    output_file = "xmake.lua"
    customize_xmake_template(template_file, output_file)
    send2trash(template_file)