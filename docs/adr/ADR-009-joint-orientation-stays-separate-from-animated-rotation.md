# Joint Orientation Stays Separate From Animated Rotation

Project Phoenix stores `jointOrientation` as rig structure data and keeps animated rotation inside `localTransform.rotation`. We chose this split because it is harder to reverse later, it is not obvious from the viewport alone, and it keeps `v0.4` skeleton authoring compatible with later `v0.5` skinning and animation workflows without mixing bind-time rig data with keyed motion.
