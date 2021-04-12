from rest_framework import serializers
from moistureduino.models import Entry


#class EntrySerializer(serializers.Serializer):
#    id = serializers.IntegerField(read_only=True)
#    created = serializers.DateTimeField()
#    kind = serializers.CharField(required=False, allow_blank=True, max_length=100)
#    value = serializers.CharField(required=False, allow_blank=True, max_length=100)
#
#    def create(self, validated_data):
#        """
#        Create and return a new `Entry` instance, given the validated data.
#        """
#        return Entry.objects.create(**validated_data)
#
#    def update(self, instance, validated_data):
#        """
#        Update and return an existing `Entry` instance, given the validated data.
#        """
#        instance.created = validated_data.get('created', instance.created)
#        instance.kind = validated_data.get('kind', instance.kind)
#        instance.value = validated_data.get('value', instance.value)
#        instance.save()
#        return instance

class EntrySerializer(serializers.ModelSerializer):
    class Meta:
        model = Entry
        fields = ['created', 'kind', 'value']
